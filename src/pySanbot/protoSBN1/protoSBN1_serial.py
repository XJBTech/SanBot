import logging
from threading import Thread, Event
from queue import Queue, Empty

import serial.tools.list_ports

from serial import SerialException, SerialTimeoutException, Serial

from protoSBN1 import protoSBN1
from util.async_event_engine import AsyncEvent, AsyncEventType, AsyncEventEngine
from device.interface import PacketListener

__author__ = 'Yifei'


def available_ports():
    ports = list(serial.tools.list_ports.comports())
    return ports


def _clear_queue(q):
    """
    Clears the queue q, this function blocks until the q is cleared
    :param q: the queue to be cleared
    :type q: queue.Queue
    :return: none
    """
    with q.mutex:
        # lock q is mandatory because it could be used elsewhere
        q.queue.clear()


class EventHandlerList(list):
    def __call__(self, *args, **kwargs):
        for f in self:
            assert callable(f)
            f(*args, **kwargs)


class ProtoSBN1SerialPortComm(object):
    __TIME_OUT = 0.1

    def __init__(self, port_name, baud_rate, proto_sbn1=protoSBN1.ProtoSBN1()):
        self._port_name = port_name
        self._baud_rate = baud_rate

        # do not open port now
        self._serial_port = Serial(
            port=None,
            baudrate=self._baud_rate,
            timeout=self.__TIME_OUT
        )
        self._serial_open = False
        self.protoSBN1 = proto_sbn1

        # initialize the LIFO queues both ways rx/tx
        self._rx_queue = Queue()
        self._tx_queue = Queue()

        # initialize the event engine
        self._async_event_engine = AsyncEventEngine()

        # initialize the threads
        self._rx_thread = self.SerialReaderThread(
            serial_port=self._serial_port,
            timeout=self.__TIME_OUT,
            protosbn1=self.protoSBN1,
            rx_packet_queue=self._rx_queue,
            event_engine=self._async_event_engine
        )

        self._tx_thread = self.SerialWriterThread(
            serial_port=self._serial_port,
            timeout=self.__TIME_OUT,
            protosbn1=self.protoSBN1,
            tx_packet_queue=self._tx_queue,
            event_engine=self._async_event_engine
        )

        self.logger = logging.getLogger(type(self).__name__)

    def _open_serial_port(self):
        self.logger.info("open serial port")
        # clears both queues to make sure we don't have random stuff in the
        # queue
        _clear_queue(self._rx_queue)
        _clear_queue(self._tx_queue)

        # initialize the serial port object before opening
        self._serial_port.port = self._port_name
        if self._serial_port.isOpen():
            self._serial_open = True
        else:
            try:
                self._serial_port.open()
            except SerialException as e:
                self.logger.error(e)
                return False
            else:
                self._serial_open = True
        return True

    def _close_serial_port(self):
        self.logger.info("close serial port")
        if not (self._serial_port.isOpen()):
            self._serial_open = False
        else:
            try:
                self._serial_port.close()
            except SerialException as e:
                self.logger.error(e)
                return False
            else:
                self._serial_open = False
        return True

    def start(self):
        self.logger.debug("starting device")
        if self._serial_port.isOpen():
            self._close_serial_port()
        self._serial_open = True
        result = self._open_serial_port()
        if not result:
            # we do not have a properly open-ed serial port
            raise SerialException
        # we should have a properly started serial port now
        self._async_event_engine.start()
        self._rx_thread.register_unplug_callback(self._unplug_callback)
        self._rx_thread.start()
        self._tx_thread.start()
        self._async_event_engine.execute_callback(
            AsyncEvent(AsyncEventType.connect_event)
        )
        self.logger.debug("started device")

    def stop(self, join=True):
        self.logger.debug("stopping device")
        self._async_event_engine.execute_callback(
            AsyncEvent(AsyncEventType.disconnect_event)
        )
        self._tx_thread.cancel(join)
        self._rx_thread.cancel(join)
        self._rx_thread.deregister_unplug_callback(self._unplug_callback)
        self._async_event_engine.cancel()
        if self._serial_port.isOpen():
            self._close_serial_port()
        self._serial_open = False
        self.logger.debug("stopped device")

    def send_packet_with_queue(self, operand, payload, block=True, timeout=0):
        packet = self.protoSBN1.create_packet(operand, payload)
        self._tx_queue.put(packet, block, timeout)

    def get_packet_from_queue(self, block=True, timeout=0):
        return self._rx_queue.get(block, timeout)

    def add_packet_listener(self, packet_listener):
        assert isinstance(packet_listener, PacketListener)
        self.register_event_handler(AsyncEventType.packet_sent_event,
                                    packet_listener.on_packet_sent)
        self.register_event_handler(AsyncEventType.packet_received_event,
                                    packet_listener.on_packet_received)

    def remove_packet_listener(self, packet_listener):
        assert isinstance(packet_listener, PacketListener)
        self.deregister_event_handler(AsyncEventType.packet_sent_event,
                                      packet_listener.on_packet_sent)
        self.deregister_event_handler(AsyncEventType.packet_received_event,
                                      packet_listener.on_packet_received)

    def register_event_handler(self, event_type, handler):
        self._async_event_engine.register_event_handler(event_type, handler)

    def deregister_event_handler(self, event_type, handler):
        self._async_event_engine.deregister_event_handler(event_type, handler)

    def _unplug_callback(self):
        self.logger.warn("serial port disconnected")
        self.stop(False)

    class SerialWriterThread(Thread):
        def __init__(self, serial_port, protosbn1,
                     tx_packet_queue, timeout, event_engine
                     ):
            """
            Initialize the writer thread

            :param serial_port: serial port (opened)
            :param protosbn1: the ProtoSBN1 instance that keeps tracks of
                                this session
            :param tx_packet_queue: the synchronized queue of ProtoSBN1Packet

            :type serial_port: serial.Serial
            :type protosbn1: protoSBN1.ProtoSBN1
            :type tx_packet_queue: queue.Queue
            :type event_engine: util.async_event_engine.AsyncEventEngine

            :return: none
            """
            super().__init__()
            self._timeout = timeout
            self._serial_port = serial_port
            self._protoSBN1 = protosbn1
            self._tx_packet_queue = tx_packet_queue
            self._cancelled = Event()
            self._event_engine = event_engine
            self.setName(type(self).__name__ + " - " + self.getName())
            self.logger = logging.getLogger(type(self).__name__)

        def run(self):
            self.logger.debug("serial writer thread started")
            self._cancelled.clear()
            while not self._cancelled.is_set():
                # try taking packet from the queue
                try:
                    packet = self._tx_packet_queue.get(block=True, timeout=0.1)
                except Empty:
                    # empty queue, we continue on this thread
                    continue
                if packet:
                    # we got a valid packet
                    assert isinstance(packet, protoSBN1.ProtoSBN1Packet)
                    try:
                        self._serial_port.write(packet.generate_bytes())
                        self._event_engine.execute_callback(
                            AsyncEvent(AsyncEventType.packet_sent_event,
                                       [[packet]])
                        )
                    except SerialException:
                        # exception in serial port, we still continue on the
                        # thread
                        continue
            self.logger.debug("serial writer thread stopped")

        def cancel(self, join=True):
            self._cancelled.set()
            if join:
                self.join()

    class SerialReaderThread(Thread):
        def __init__(self, serial_port, protosbn1,
                     rx_packet_queue, timeout, event_engine
                     ):
            """
            Initialize the reader thread

            :param serial_port: serial port (opened)
            :param protosbn1: the ProtoSBN1 instance that keeps tracks of
                                this session
            :param rx_packet_queue: the synchronized queue of ProtoSBN1Packet

            :type serial_port: serial.Serial
            :type protosbn1: protoSBN1.ProtoSBN1
            :type rx_packet_queue: queue.Queue
            :type event_engine: util.async_event_engine.AsyncEventEngine

            :return: None
            """
            super().__init__()
            self._timeout = timeout
            self._serial_port = serial_port
            self._protoSBN1 = protosbn1
            self._rx_packet_queue = rx_packet_queue
            self._unplug_event = EventHandlerList()
            self._event_engine = event_engine
            self._cancelled = Event()
            self.setName(type(self).__name__ + " - " + self.getName())
            self.logger = logging.getLogger(type(self).__name__)

        def register_unplug_callback(self, callback):
            self._unplug_event.append(callback)

        def deregister_unplug_callback(self, callback):
            self._unplug_event.remove(callback)

        def run(self):
            self.logger.debug("serial reader thread started")
            self._cancelled.clear()
            data = []
            serial_unplugged = False
            while not self._cancelled.is_set():
                incoming_data = None
                try:
                    incoming_data = self._serial_port.read(
                        self._serial_port.inWaiting() or 1
                    )
                except SerialTimeoutException:
                    # time out on serial read, retry
                    if not incoming_data:
                        # skip when we haven't got any data
                        continue
                except SerialException:
                    # serial port unplug or force closed
                    if self.isAlive:
                        serial_unplugged = True
                    else:
                        serial_unplugged = False
                    break
                if incoming_data:
                    # the data is valid from serial port
                    # append the data to a local buffer
                    data += list(incoming_data)
                    parsed_packets, consumed_bytes = \
                        self._protoSBN1.parse(data)
                    # clear the number of bytes used
                    data = data[consumed_bytes:]
                    # add all the parsed packets into queue
                    if parsed_packets:
                        # we have at least one valid packet
                        list(map(lambda packet:
                                 self._rx_packet_queue.put(
                                     item=packet,
                                     block=True,
                                     timeout=self._timeout
                                 ), parsed_packets))
                        self._event_engine.execute_callback(
                            AsyncEvent(AsyncEventType.packet_received_event,
                                       [parsed_packets])
                        )

            if serial_unplugged:
                # we need to signal a serial port unplugged event here
                self.logger.debug("serial port is unplugged, calling "
                                  "_disconnect_event")
                self._unplug_event()
            self.logger.debug("serial reader thread stopped")

        def cancel(self, join=True):
            self._cancelled.set()
            if join:
                self.join()
