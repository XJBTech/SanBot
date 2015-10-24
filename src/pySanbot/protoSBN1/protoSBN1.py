from util import byte_bit_util
from util.boyer_moore_search import BoyerMoore, boyer_moore
from protoSBN1 import protoSBN1_constants
from protoSBN1.protoSBN1_operand import ProtoSBN1Operand

__author__ = 'Yifei'


class ProtoSBN1(object):
    """ProtoSBN1 class"""

    def __init__(self):
        self.__rx_index = 0
        self.__tx_index = 0

    def get_probe_request_packet(self):
        payload_bytes = []
        return self.create_packet(ProtoSBN1Operand.probe_request, payload_bytes)

    def create_packet(self, operand, payload):
        packet = ProtoSBN1Packet(self.__tx_index, operand, payload)
        self.__tx_index = (self.__tx_index + 1) % 256
        return packet

    def generate_probe_request(self):
        return self.create_packet(
            operand=ProtoSBN1Operand.probe_request,
            payload=[]
        )

    def parse(self, buffer, verify_index=True, update_index=True):
        packet_list = []
        original_buffer_length = len(buffer)
        byte_consumed = 0

        # header structure of possible packet
        index_header = 0
        index_header_iter = 0
        index = 0
        payload = None
        operand = None

        found = False

        if len(buffer) < protoSBN1_constants.MIN_PACKET_LENGTH:
            return packet_list, byte_consumed

        p_bm = BoyerMoore(protoSBN1_constants.PROTOSBN1_HEADER_BYTES)
        index_header_occurrences = boyer_moore(
            protoSBN1_constants.PROTOSBN1_HEADER_BYTES, p_bm,
            buffer)
        index_header_occurrences_num = len(index_header_occurrences)

        while True:
            # Find next header
            if index_header_iter == index_header_occurrences_num:
                # we do not have an header next
                byte_consumed = original_buffer_length
                break
            index_header = index_header_occurrences[index_header_iter]
            index_header_iter += 1

            # We have a header now
            if original_buffer_length - index_header < protoSBN1_constants.MIN_PACKET_LENGTH:
                byte_consumed = index_header
                break

            # Check length
            # expected packet length
            # = len(op+payload) + 4 (SBN1) + 1 (index) + 1 (len) + 1 (checksum)
            expected_packet_length = \
                buffer[
                    index_header + protoSBN1_constants.LENGTH_OFFSET] \
                + protoSBN1_constants.MIN_PACKET_LENGTH
            # Packet has a valid header, check payload length
            if original_buffer_length - index_header < expected_packet_length:
                # The packet is not long enough, keep these bytes
                byte_consumed = index_header
                break
            elif expected_packet_length == protoSBN1_constants.MIN_PACKET_LENGTH:
                # Zero length packet, drop
                byte_consumed = index_header + protoSBN1_constants.MIN_PACKET_LENGTH
                continue

            # Check checksum
            # We are separating operand and payload
            payload = [0] * (buffer[
                                 index_header + protoSBN1_constants.LENGTH_OFFSET] - 1)
            payload = buffer[
                      index_header + protoSBN1_constants.PAYLOAD_OFFSET:index_header + protoSBN1_constants.PAYLOAD_OFFSET + len(
                          payload)]
            checksum = buffer[
                index_header + protoSBN1_constants.CHECKSUM_OFFSET]
            operand = ProtoSBN1Operand(buffer[
                                           index_header + protoSBN1_constants.OPERAND_OFFSET])
            checksum_correct = self.verify_checksum(operand, payload, checksum)
            # If the checksum is not correct, consume the header and move on
            if not checksum_correct:
                byte_consumed = index_header + len(
                    protoSBN1_constants.PROTOSBN1_HEADER_BYTES)
                continue

            # Verify index
            index = buffer[
                index_header + protoSBN1_constants.INDEX_OFFSET]
            if verify_index:
                index_correct = self.verify_rx_index(index)
                if index_correct:
                    found = True
                else:
                    byte_consumed = index_header + protoSBN1_constants.MIN_PACKET_LENGTH
            elif update_index:
                # Update index accordingly
                self.__rx_index = (index + 1) % 256
                found = True

            # Found a valid packet
            if found:
                packet = ProtoSBN1Packet(index, operand, payload)
                packet_list.append(packet)
                found = False
                byte_consumed = index_header + len(packet.generate_bytes())
        return packet_list, byte_consumed

    @staticmethod
    def verify_checksum(operand, payload, checksum):
        return checksum == ((sum(payload) + operand.value) % 256)

    def verify_rx_index(self, incoming_index):
        if incoming_index != self.__rx_index:
            # Incorrect index, not in sync
            # update _rxIndex for next packet
            self.__rx_index = (incoming_index + 1) % 256
            return False
        self.__rx_index = (self.__rx_index + 1) % 256
        return True


class ProtoSBN1Packet(object):
    """Packet for protoSBN1"""

    def __init__(self, index, operand, payload):
        """Initialize the packet"""
        self._index = index
        self._length = len(payload) + 1
        self._operand = operand
        self._payload = payload
        self._checksum = 0
        self.update_checksum()
        self._bytes = None

    def operand(self):
        return self._operand

    def payload(self):
        return self._payload

    def update_checksum(self):
        """Calculate checksum for this packet
        :returns: The checksum for this packet
        """
        self._checksum = (self._operand.value + sum(self._payload)) % 256
        return self._checksum

    def generate_bytes(self):
        """Generate bytes for the ProtoSBN1Packet
        :returns: The byte array for this ProtoSBN1Packet
        """
        if self._bytes is None:
            # generate bytes
            self._bytes = protoSBN1_constants.PROTOSBN1_HEADER_BYTES + \
                          [self._index, self._length, self._checksum,
                           self._operand.value] + \
                          self._payload
        return self._bytes

    def __str__(self):
        return byte_bit_util.byte_list_to_string(self.generate_bytes())

    def __eq__(self, other):
        if not (type(other) is type(self)):
            return False
        if str(self) == str(other):
            return True
        return False

    def __ne__(self, other):
        return not (self.__eq__(other))
