__author__ = 'Kalmar'
__version__ = '0.1'

from socket import socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP, timeout
from struct import pack, unpack


class Conversation(object):
    sock = None
    pin = None
    player_number = None
    fps = None
    command_set = dict(MOVE_RIGHT=0, MOVE_LEFT=1, MOVE_UP=2, MOVE_DOWN=3, ROT_RIGHT=4, ROT_LEFT=5, SHOOT=6)
    flags = dict(ALIVE=0, SHOT=1)

    def __init__(self, settings):
        self.pin = pack("I", settings['ID'])
        self.player_number = settings['NR']
        self.fps = settings['FPS']
        self.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
        self.sock.connect((settings['IP'], settings['PORT']))
        self.sock.settimeout(1.0 / self.fps * 2)

    def parse_server(self, buf):
        n = ord(buf[0])
        players = []
        for i in xrange(n):
            tmp = buf[(1+12*i):(1+12*(i+1))]
            dic = dict()
            dic['x'] = unpack("H", tmp[:2])[0]
            dic['y'] = unpack("H", tmp[2:4])[0]
            dic['angle'] = unpack("f", tmp[4:8])[0]
            dic['hp'] = unpack("H", tmp[8:10])[0]
            flags = unpack("H", tmp[10:12])[0]
            dic['alive'] = (((flags >> self.flags["ALIVE"]) & 1) == 1)
            dic['shot'] = (((flags >> self.flags["SHOT"]) & 1) == 1)
            players.append(dic)
        return players

    def parse_client(self, commands):
        buf = self.pin
        com = 0
        for i in commands:
            try:
                com |= (1 << self.command_set[i])
            except KeyError:
                print "parse_client: wrong command!"
        return buf+chr(com & 0xff)

    def get_board(self):
        try:
            buf = self.sock.recv(4096)
            return self.parse_server(buf)
        except timeout:
            print "get_board: connection timed out."
            return None

    def send_commands(self, commands=()):
        self.sock.sendall(self.parse_client(commands))

    def hello(self):
        self.send_commands()