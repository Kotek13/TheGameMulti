__author__ = ''

import settings
from combat import Combat, ActionFinished
from conversation import Conversation
import random

DEBUG_BOT = False


class Bot(object):
    server = None
    players = []
    command = 0
    commands = []
    combat = None
    connected = False
    reloading = False
    counter = 0
    ammo = settings.AMMO

    def __init__(self, server):
        self.server = server

    def handle_commands(self):
        self.command = 0
        for i in self.commands:
            try:
                result = i[0](*i[1])
                if isinstance(result, int):
                    self.command |= result
                else:
                    self.commands[:] = [x for x in self.commands if x != i]
            except ActionFinished as e:
                self.commands[:] = [x for x in self.commands if x != i]
                if settings.DEBUG or DEBUG_BOT:
                    print(e)
            except AttributeError:
                self.commands[:] = [x for x in self.commands if x != i]

    def is_there(self, command, args=None):
        for i in xrange(len(self.commands)):
            if args:
                if [command, args] == self.commands[i]:
                    return i
            else:
                if self.commands[i][0] == command:
                    return i
        return -1

    def move(self, direction, percentage=1.0):
        assert isinstance(direction, str)
        assert isinstance(percentage, float)
        if self.is_there(self.combat.move, [direction.upper(), percentage]) == -1:
            self.commands.append([self.combat.move, [direction.upper(), percentage]])

    def kill_some_player(self):
        if self.is_there(self.combat.kill) == -1:
            player = settings.MY_NR
            while player == settings.MY_NR or not self.players[player].alive:
                player = random.randint(0, len(self.players) - 1)
            self.commands.append([self.combat.kill, [player]])

    def algorithm(self):
        #self.move("RIGHT", 0.5)
        self.kill_some_player()
        self.handle_commands()

    def run(self):
        self.server.hello()
        self.combat = Combat(self.server)
        while True:
            self.players = self.server.get_players(self.players)
            if self.players:
                self.combat.update(self.players)
                self.algorithm()
                self.server.send_command(self.command)

if __name__ == '__main__':
    import sys
    settings.IP = sys.argv[1]
    settings.MY_NR = int(sys.argv[2], 10)
    settings.PIN = int(sys.argv[3], 16)
    Bot(Conversation()).run()