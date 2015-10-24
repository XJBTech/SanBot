"""
File: event_engine.py
Author: Yifei Zhang
Email: njzhangyifei@gmail.com
Github: https://github.com/njzhangyifei
Description: An asynchronous event engine
"""

from enum import Enum
from queue import Queue, Empty
from threading import Thread, Event, Lock

__author__ = 'Yifei'


class AsyncEventType(Enum):
    """
    Types of Asynchronous Event
    """
    connect_event = 1
    disconnect_event = 2
    packet_received_event = 3
    packet_sent_event = 4


class AsyncEvent(object):
    """
    Asynchronous Event object, this object allows passing any number of list
    or keyword params into the callback
    """

    def __init__(self, event_type, args=None, kwargs=None):
        self.type = event_type
        self.args = args
        self.kwargs = kwargs


class AsyncEventHandlerList(list):
    """
    A class derived from list to hold all the event handlers
    """

    def __call__(self, *args, **kwargs):
        for f in self:
            assert callable(f)
            f(*args, **kwargs)


class AsyncEventEngine(Thread):
    """
    Async Event Engine class, derived from Thread
    """
    __TIME_OUT = 0.1

    def __init__(self, timeout=__TIME_OUT):
        """
        Initialize the class with a given timeout on queue
        :param timeout: the timeout on queue
        :return: None
        """
        super().__init__()
        self._timeout = timeout
        self._cancelled = Event()
        self._event_queue = Queue()
        self._event_handlers_lock = Lock()
        self._event_handlers = {}

    def run(self):
        """
        Overrides the Thread.run() method
        :return: None
        """
        self._cancelled.clear()
        while not self._cancelled.is_set():
            try:
                pending_event = self._event_queue.get(block=True,
                                                      timeout=self._timeout)
            except Empty:
                continue
            if pending_event:
                assert isinstance(pending_event, AsyncEvent)
                event_handlers = None
                with self._event_handlers_lock:
                    if pending_event.type in self._event_handlers.keys():
                        event_handlers = self._event_handlers[
                            pending_event.type]
                if event_handlers:
                    if pending_event.kwargs:
                        # use keyword arguments
                        event_handlers(**pending_event.kwargs)
                    elif pending_event.args:
                        # use arguments
                        event_handlers(*pending_event.args)
                    else:
                        # otherwise do not use any arguments
                        event_handlers()

    def cancel(self, join=True, timeout=0):
        """
        Cancel this thread
        :param join: whether to join this thread
        :param timeout: timeout on joining the thread
        :return: None
        """
        # cancel this event engine
        self._cancelled.set()
        with self._event_queue.mutex:
            self._event_queue.queue.clear()
        with self._event_handlers_lock:
            self._event_handlers.clear()
        if join:
            self.join(timeout=timeout)

    def register_event_handler(self, event_type, handler):
        """
        Register an event handler into the event engine
        :param event_type: type of the event
        :param handler: a callable object which will be called on event happens
        :return: None
        """
        with self._event_handlers_lock:
            # locking event_handler dictionary
            if not (event_type in self._event_handlers.keys()):
                self._event_handlers[event_type] = AsyncEventHandlerList()
            self._event_handlers[event_type].append(handler)

    def deregister_event_handler(self, event_type, handler):
        """
        Remove a given handler from the handler list of a given event type
        :param event_type: type of the event
        :param handler: the callable object to remove
        :return: None
        """
        with self._event_handlers_lock:
            if event_type in self._event_handlers.keys():
                if handler in self._event_handlers[event_type]:
                    self._event_handlers[event_type].remove(handler)

    def execute_callback(self, event):
        """
        Adds the event into a queue for dispatch, will block until added
        :param event: a AsyncEvent object which specifies event types and args
        :return: None
        """
        self._event_queue.put(event)
