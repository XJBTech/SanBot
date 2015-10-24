"""
File: protoSBN1_operand.py
Author: Yifei Zhang
Email: njzhangyifei@gmail.com
Github: https://github.com/njzhangyifei
Description: Contains all the operand for ProtoSBN1
"""

from enum import Enum

__author__ = 'Yifei'


class ProtoSBN1Operand(Enum):
    probe_request = 0x01
    probe_response = 0x02
    search_request = 0x03
    search_response = 0x04
    read_request = 0x05
    read_response = 0x06
    lock_request = 0x07
    lock_response = 0x08
    self_check_request = 0x09
    self_check_response = 0x0A
    device_info_request = 0x0B
    device_info_response = 0x0C


