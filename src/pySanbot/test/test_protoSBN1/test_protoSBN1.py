from unittest import TestCase
from protoSBN1.protoSBN1_operand import ProtoSBN1Operand
from protoSBN1.protoSBN1 import ProtoSBN1, ProtoSBN1Packet
from protoSBN1.protoSBN1_constants import *

__author__ = 'Yifei'


class TestProtoSBN1(TestCase):
    def test_get_probe_request_packet(self):
        proto_sbn1 = ProtoSBN1()
        for i in range(0, 1000):
            probe_request_packet = proto_sbn1.get_probe_request_packet()
            expected_packet = PROTOSBN1_HEADER_BYTES + \
                              [(0x00 + i) % 256, 0x01, 0x01,
                               ProtoSBN1Operand.probe_request.value]
            self.assertLessEqual((0x00 + i) % 256, 255)
            self.assertEqual(expected_packet,
                             probe_request_packet.generate_bytes())

    def test_parse_valid_packet(self):
        # Test Case 0: Single Normal Packet
        proto_sbn1 = ProtoSBN1()

        payload = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                   0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c]
        index = 0x00
        operand = ProtoSBN1Operand.probe_response
        checksum = sum(payload) + operand.value

        buffer = PROTOSBN1_HEADER_BYTES + \
                 [index, len(payload) + 1, checksum, operand.value] + payload
        parsed_packet_list, byte_consumed = proto_sbn1.parse(buffer)

        expected_packet = ProtoSBN1Packet(index, operand, payload)

        self.assertEqual(1, len(parsed_packet_list),
                         "Test Case 0: Normal Packet")
        self.assertEqual(expected_packet, parsed_packet_list[0],
                         "Test Case 0: Normal Packet")

        # Test Case 1: Two continued Normal Packet
        index = 0x01
        checksum = sum(payload) + operand.value
        buffer = PROTOSBN1_HEADER_BYTES + \
                 [index, len(payload) + 1, checksum, operand.value] + payload
        expected_packet_0 = ProtoSBN1Packet(index, operand, payload)
        index = 0x02
        checksum = sum(payload) + operand.value
        buffer += PROTOSBN1_HEADER_BYTES + \
                  [index, len(payload) + 1, checksum, operand.value] + payload
        expected_packet_1 = ProtoSBN1Packet(index, operand, payload)

        parsed_packet_list, byte_consumed = proto_sbn1.parse(buffer)

        self.assertEqual(2, len(parsed_packet_list),
                         "Test Case 1: Continued-Index Packets")
        self.assertEqual(expected_packet_0, parsed_packet_list[0],
                         "Test Case 1: Continued-Index Packets")
        self.assertEqual(expected_packet_1, parsed_packet_list[1],
                         "Test Case 1: Continued-Index Packets")

    def test_parse_valid_packet_with_dummy_data(self):
        # Test Case 0
        test_case_message = \
            "Test Case 0: Valid Packets with Dummy Data in between"
        proto_sbn1 = ProtoSBN1()
        dummy_data = [0x00, 0x10, 0x09, 0x05, 0x12]

        # First Packet
        payload = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                   0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c]
        index = 0x00
        operand = ProtoSBN1Operand.probe_response
        checksum = sum(payload) + operand.value
        expected_packet_0 = ProtoSBN1Packet(index, operand, payload)
        buffer = PROTOSBN1_HEADER_BYTES + \
                 [index, len(payload) + 1, checksum, operand.value] + payload

        # Adds dummy data
        buffer += dummy_data

        # Second Packet
        index = 0x01
        checksum = sum(payload) + operand.value
        buffer += PROTOSBN1_HEADER_BYTES + \
                  [index, len(payload) + 1, checksum, operand.value] + payload
        expected_packet_1 = ProtoSBN1Packet(index, operand, payload)

        # Parse
        parsed_packet_list, byte_consumed = proto_sbn1.parse(buffer)

        self.assertEqual(2, len(parsed_packet_list), test_case_message)
        self.assertEqual(expected_packet_0, parsed_packet_list[0],
                         test_case_message)
        self.assertEqual(expected_packet_1, parsed_packet_list[1],
                         test_case_message)

        # Test Case 1
        test_case_message = \
            "Test Case 1: Valid Packets with Dummy Data in between and begin end"
        # Create new data
        buffer = dummy_data + buffer + dummy_data

        # Reinitialize to reset index
        proto_sbn1 = ProtoSBN1()

        # Parse

        # Parse
        parsed_packet_list, byte_consumed = proto_sbn1.parse(buffer)

        self.assertEqual(2, len(parsed_packet_list), test_case_message)
        self.assertEqual(expected_packet_0, parsed_packet_list[0],
                         test_case_message)
        self.assertEqual(expected_packet_1, parsed_packet_list[1],
                         test_case_message)

    def test_parse_valid_probe_response(self):
        data = [83, 66, 78, 49, 0, 14, 108, 2, 1, 84, 255, 115, 6, 80, 119, 83,
              81, 84, 81, 6, 135]
        proto_sbn1 = ProtoSBN1()
        parsed_packet_list, byte_consumed = proto_sbn1.parse(data)
        self.assertEqual(1, len(parsed_packet_list))
        self.assertEqual(ProtoSBN1Operand.probe_response, parsed_packet_list[
            0]._operand)
        self.assertEqual(len(data), byte_consumed)

