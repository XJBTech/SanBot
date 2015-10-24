__author__ = 'Yifei'


class PacketListener(object):
    def on_packet_received(self, packets):
        raise NotImplementedError

    def on_packet_sent(self, packets):
        raise NotImplementedError


class ConnectionListener(object):
    def on_connected(self, packets):
        raise NotImplementedError

    def on_disconnected(self, packets):
        raise NotImplementedError


