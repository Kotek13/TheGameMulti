__author__ = 'Kalmar'
__version__ = '0.1'

from socket import socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP, timeout
from struct import pack, unpack
from math import cos, sin
import settings


class Bullet(object):
    x = 0
    y = 0
    v_x = 0
    v_y = 0
    owner_nr = None

    def __init__(self, player):
        self.owner_nr = player.player_nr
        self.x = int(player.x + 1.0 * settings.BLOCK_SIZE / 2 + cos(player.angle) * settings.BLOCK_SIZE / 2 * settings.GUN_SIZE - 1)
        self.y = int(player.y + 1.0 * settings.BLOCK_SIZE / 2 - sin(player.angle) * settings.BLOCK_SIZE / 2 * settings.GUN_SIZE - 1)
        self.v_x = int(settings.BULLET_SPEED * cos(player.angle))
        self.v_y = 0 - int(settings.BULLET_SPEED * sin(player.angle))

    def move(self):
        self.x += self.v_x
        self.y += self.v_y


class Player(object):
    x = None
    y = None
    v_x = 0
    v_y = 0
    angle = None
    hp = settings.HP
    ammo = settings.AMMO
    alive = True
    shot = False
    player_nr = None

    def shoot(self):
        return Bullet(self)


class Conversation(object):
    sock = None
    pin = None
    command_set = dict(MOVE_RIGHT=0, MOVE_LEFT=1, MOVE_UP=2, MOVE_DOWN=3, ROT_RIGHT=4, ROT_LEFT=5, SHOOT=6)
    flags = dict(ALIVE=0, SHOT=1)

    def __init__(self):
        self.pin = pack("I", settings.PIN)
        self.fps = settings.FPS
        self.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
        self.sock.connect((settings.IP, settings.PORT))
        self.sock.settimeout(1.0 / settings.FPS * 2)

    def parse_server(self, buf):
        n = ord(buf[0])
        players = []
        for i in xrange(n):
            tmp = buf[(1+12*i):(1+12*(i+1))]
            pl = Player()
            pl.player_nr = i
            pl.x = unpack("H", tmp[:2])[0]
            pl.y = unpack("H", tmp[2:4])[0]
            pl.angle = unpack("f", tmp[4:8])[0]
            pl.hp = unpack("H", tmp[8:10])[0]
            flags = unpack("H", tmp[10:12])[0]
            pl.alive = (((flags >> self.flags["ALIVE"]) & 1) == 1)
            pl.shot = (((flags >> self.flags["SHOT"]) & 1) == 1)
            players.append(pl)
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

    def get_players(self):
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