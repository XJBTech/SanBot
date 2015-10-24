import logging
from queue import Empty
from multiprocessing import Pool, cpu_count
from serial.tools import list_ports
from serial import SerialException
from protoSBN1.protoSBN1 import ProtoSBN1Packet
from protoSBN1.protoSBN1_serial import ProtoSBN1SerialPortComm
from protoSBN1.protoSBN1_operand import ProtoSBN1Operand
from device.interface import PacketListener

__author__ = 'Yifei'


class Dongle(PacketListener):
    __BAUD_RATE = 921600
    __TIMEOUT_QUEUE = 0.05

    def __init__(self, probe_result_entry=None, port_name=None):
        if probe_result_entry:
            self._protoSBN1_serial = ProtoSBN1SerialPortComm(
                port_name=probe_result_entry[0],
                baud_rate=self.__BAUD_RATE,
                proto_sbn1=probe_result_entry[2]
            )
        else:
            self._protoSBN1_serial = ProtoSBN1SerialPortComm(
                port_name=port_name,
                baud_rate=self.__BAUD_RATE
            )
        self._timeout_queue = self.__TIMEOUT_QUEUE
        self.logger = logging.getLogger(type(self).__name__)

    def on_packet_sent(self, packets):
        pass

    def on_packet_received(self, packets):
        pass

    def start(self):
        self._protoSBN1_serial.add_packet_listener(self)
        self._protoSBN1_serial.start()

    def stop(self):
        self._protoSBN1_serial.stop()
        self._protoSBN1_serial.remove_packet_listener(self)

    def send_probe_packet(self):
        """
        Send probe request to the underlying serial (not that useful though)
        :return: None
        """
        self._protoSBN1_serial.send_packet_with_queue(
            ProtoSBN1Operand.probe_request, [])

    @classmethod
    def _probe_for_dongle(cls, port_entry):
        baud_rate = cls.__BAUD_RATE
        port_name = port_entry[0]
        trials = 0
        result = False
        serial_port = ProtoSBN1SerialPortComm(port_name, baud_rate)
        try:
            serial_port.start()
        except SerialException:
            # exception in opening port, not a possible valid dongle
            return result
        while trials < 10:
            serial_port.send_packet_with_queue(
                ProtoSBN1Operand.probe_request, [])
            while True:
                try:
                    packet = serial_port.get_packet_from_queue(
                        block=True, timeout=cls.__TIMEOUT_QUEUE)
                except Empty:
                    # we don't have a packet yet
                    break
                if packet:
                    assert isinstance(packet, ProtoSBN1Packet)
                    if packet.operand() == ProtoSBN1Operand.probe_response:
                        # correct response
                        result = True
                        break
                    else:
                        # not a response we would like to see
                        continue
                else:
                    # not a packet
                    continue
            if result:
                # we do have a valid response from the device
                break
            trials += 1
        # remember to stop the port
        serial_port.stop()
        return result, serial_port.protoSBN1

    @classmethod
    def probe_for_dongles(cls):
        """
        Probe all available serial ports
        :return: a list of tuples describing the serial port, tuple[0]
        represents whether a possible dongle is there
        """
        ports = list(list_ports.comports())
        processes_pool = Pool(cpu_count())
        results = processes_pool.map(cls._probe_for_dongle, ports)
        processes_pool.close()
        return [(detail[0],) + result + detail[1:] for detail, result in zip(
            ports, results)]
