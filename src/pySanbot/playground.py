__author__ = 'Yifei'

import time
from device.dongle import Dongle
from protoSBN1.protoSBN1_operand import ProtoSBN1Operand

if __name__ == '__main__':
    results = Dongle.probe_for_dongles()
    d = Dongle(results[0])
    d.start()
    t = 0
    while t < 10:
        print("sending {0}-th probe", t)
        d.send_probe_packet()
        time.sleep(0.01)
        t += 1

    d.stop()




