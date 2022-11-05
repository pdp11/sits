#!/usr/bin/env python3

import sys
import socket

class State:
    state = None
    sock = None

    def tyi():
        return State.sock.recv(1)

    def tyo(b):
        if b is not None:
            State.sock.send(b)

    def word(data):
        tyo((data & 0o77) + 0o40)
        tyo(((data >> 6) & 0o17) + 0o40)
        tyo(((data >> 10) & 0o77) + 0o40)

    def type(s):
        for c in s:
            State.tyo(c.encode())

    def crlf():
        State.type("\r\n")

class Hactrn:
    table = None
    prefix = ""
    jcl = None
    colon = None

def nltl2():
    Hactrn.table = optab0
    Hactrn.prefix = ""
    State.crlf()
    State.type("*")

def logout():
    data = State.tyi()
    if data == b'\032':
        State.type("AI ITS.1234. DDT.4321.\r\n")
        State.type("TTY 21\r\n")
        State.state = hactrn
        Hactrn.table = optab0

def actrlk(data):
    print("hactrn ^K")
    State.type("^K")
    State.crlf()
    Hactrn.prefix = Hactrn.prefix.upper()
    if Hactrn.prefix == "NLODER":
        State.state = nloder
    else:
        State.type("SYS: SYS3; TS " + Hactrn.prefix + " - FILE NOT FOUND")
        nltl2()

def ncart(data):
    print("hactrn ^M")
    nltl2()

def gopalt(data):
    print("hactrn altmode")
    State.type("$")
    if Hactrn.table == optab0:
        Hactrn.table = optab1
    else:
        Hactrn.table = optab2

def hactrn_prefix(data):
    State.tyo(data)
    Hactrn.prefix += data.decode()
    print("hactrn prefix: " + Hactrn.prefix)

def naltu(data):
    print("hactrn $U")
    State.tyo(data)
    nltl2()

def hactrn_logout(data):
    print("hactrn $$U")
    State.tyo(data)
    State.crlf()
    State.type("AI ITS 1234  Console 21 free.\r\n")
    State.state = logout

def ncom(data):
    Hactrn.prefix = ""
    Hactrn.jcl = None
    Hactrn.colon = hactrn_prefix
    Hactrn.table = dispatch_colon

optab0 = {
    b'\013' : actrlk,
    b'\015' : ncart,
    b'\033' : gopalt,
}

def colon_login():
    naltu(None)

def colon_logout():
    hactrn_logout(None)

def colon_nloder():
    State.state = nloder

nctab = {
    "LOGIN" : colon_login,
    "LOGOUT" : colon_logout,
    "NLODER" : colon_nloder
}

def colon_control_m(data):
    print("colon ^M")
    Hactrn.prefix = Hactrn.prefix.upper()
    if Hactrn.prefix in nctab:
        nctab[Hactrn.prefix]()
    else:
        State.type("SYS: SYS3; TS " + Hactrn.prefix + " - FILE NOT FOUND")
        nltl2()

def colon_space(data):
    State.tyo(data)
    Hactrn.colon = colon_jcl
    Hactrn.jcl = ""
    print("colon space")

def colon_jcl(data):
    State.tyo(data)
    Hactrn.jcl += data.decode()
    print("jcl: " + Hactrn.jcl)

def colon_prefix(data):
    Hactrn.colon(data)

dispatch_colon = {
    b'\015' : colon_control_m,
}

optab1 = {
    b'\033' : gopalt,
    b'u' : naltu,
    b'U' : naltu
}

optab2 = {
    b'u' : hactrn_logout,
    b'U' : hactrn_logout
}

def hactrn():
    data = State.tyi()
    if data in Hactrn.table:
        Hactrn.table[data](data)

class Palx:
    def __init__(name):
        self.f = open(name, "rb")

    def byte():
        data = self.f.read(1)
        checksum += data
        checksum &= 0o377
        return data

    def bytes(n):
        a = []
        for x in range(n):
            a.append(byte())
        return a

    def word():
        return byte() | (byte() << 8)

    def block():
        self.checksum = 0
        count = word()
        return (count,
                word(),
                bytes(count - 6),
                byte())

def block(n, a, b, c):
    State.tyo(b'\041')
    State.tyo(b'\040')
    State.word(n)
    State.word(a)
    for x1, x2 in zip(b[0::2], b[1::2]):
        State.word(x1 | (x2 << 8))
    State.word(c)
    data = State.tyi()

def nloder():
    State.type("*")
    palx = Palx("bin/its.bin")
    State.tyo(b'\001')

    while True:
        n, a, b, c = palx.block()
        block(n, a, b, c)
        if n == 6:
            break

    # get 5
    # symbols

    State.state = logout

def main():
    State.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('localhost', 20003)
    State.sock.connect(server_address)
    try:
        while True:
            State.state()
    finally:
        State.sock.close()

if __name__ == "__main__":
    State.state = logout
    for i in range(32, 126):
        optab0[i.to_bytes(1, 'big')] = hactrn_prefix
        dispatch_colon[i.to_bytes(1, 'big')] = colon_prefix
    optab0[b':'] = ncom
    dispatch_colon[b' '] = colon_space
    main()
