import random
import math


def test(path):
    fdata = open(path, "w")

    flow = random.randrange(0, 100000)
    login = random.randrange(0, 10)
    fail = random.randrange(0, login + 1)
    ports = random.randrange(0, 15)
    tcp = random.randrange(3, 15)
    start = random.randrange(0, 86399)
    end = random.randrange(start + 1, 86400)

    speed = math.ceil(flow / (end - start))

    fdata.writelines(str(flow) + "," + str(speed) + "," + str(login) + "," + str(fail) + "," + str(
        ports) + "," + str(tcp) + "," + str(start) + "," + str(end) + "\n")

    fdata.close()
    return fdata
