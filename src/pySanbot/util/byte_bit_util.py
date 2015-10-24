"""
File: ByteBitUtil.py
Author: Yifei Zhang
Email: njzhangyifei@gmail.com
Github: https://github.com/njzhangyifei
Description: Byte/Bit related helper functions
"""

__author__ = 'Yifei'


def byte_list_to_string(byte_list):
    """Convert a list to human readable string in HEX

    :returns: a string representing the list

    """
    hex_str = [hex(x) for x in byte_list]
    return "[{0}]".format(' '.join(hex_str))


def extract_bit(byte, pos):
    """Extract a single bit from a byte

    :byte: the byte from which to extract
    :pos: the position of the bit to be extracted, counting from LSB
    :returns: whether the bit is a one

    """
    return (byte & (1 << pos)) != 0


def extract_bits(byte, num, start):
    """Extract a given number of bits with the start position from byte

    :byte: the byte from which to extract
    :num: the number of bits to extract
    :start: the position from which to extracted, counting from LSB
    :returns: the extracted bits expressed in a list of bools

    """
    if num > 8:
        # we should never encounter this
        raise ValueError
    rtn = [0] * num
    for i in range(num):
        rtn[num - i - 1] = extract_bit(byte, i + start)
    return rtn


def generate_byte(bools):
    """Generate a byte based on a list of bools

    :bools: a list of bools from which to generate the byte
    :returns: a byte representing the bools

    """
    bools_len = len(bools)
    if bools_len > 8:
        # we should never encounter this, wrong length
        raise ValueError
    rtn = 0  # initialize the bool we are returning
    for i in range(bools_len):
        rtn += ((1 if bools[bools_len - i - 1] else 0) << i)
    return rtn
