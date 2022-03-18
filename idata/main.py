import random

count = 512


def genfloats():
    fout = open('fdata.data', 'w')
    limits = [0.111, 10000.333, 100000.55555, 1000000.7777777]
    for cnt in range(0, count):
        for i in limits:
            num = random.uniform(0.00, i)
            fout.write('data(')
            fout.write('{:.6f}, '.format(num))
            fout.write('{:.16f}, '.format(num))
            fout.write(float.hex(num))
            fout.write(', {:e}'.format(num))
            fout.write('),\n')
    fout.close()


def genints():
    fout = open('idata.data', 'w')
    limits = [0x7, 0x7F, 0x7FFF, 0x7FFFFFFF, 0x7FFFFFFFFFFFFFFF]
    for cnt in range(0, count):
        for i in limits:
            num = random.randint(0, i)
            fout.write('data(')
            fout.write('{:d}, '.format(num))
            fout.write('0b{0:b}, '.format(num))
            fout.write('0{0:o}, '.format(num))
            fout.write('0x{:02x}'.format(num))
            fout.write('),\n')
    fout.close()


def genuints():
    fout = open('udata.data', 'w')
    limits = [0xF, 0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF]
    for cnt in range(0, count):
        for i in limits:
            num = random.randint(0, i)
            fout.write('data(')
            fout.write('{:d}, '.format(num))
            fout.write('0b{0:b}, '.format(num))
            fout.write('0{0:o}, '.format(num))
            fout.write('0x{:02x}'.format(num))
            fout.write('),\n')
    fout.close()


genfloats()
genints()
genuints()

print("ok")
