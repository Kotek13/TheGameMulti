from math import atan2, fabs, pi
import settings


class Simple(object):  # to be continued...
    server = None
    players = []
    command = 0
    connected = False
    reloading = False
    counter = 0
    ammo = settings.AMMO

    def __init__(self, server):
        self.server = server