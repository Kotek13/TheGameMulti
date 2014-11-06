__author__ = 'Kalmar'

from math import fabs
import settings
from combat import Combat, ActionFinished
from conversation import Conversation


DEBUG_BOT = False


class Bot(object):  # to be continued...
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

    def move_randomly(self, percentage=1.0):
        assert isinstance(percentage, float)
        if self.is_there(self.combat.random_movement, [percentage]) == -1:
            self.commands.append([self.combat.random_movement, [percentage]])

    def kill_lowest_hp(self):
        lowest_other = 0

        for i in sorted(self.players, key=lambda x: x.hp):
            if i.player_nr != settings.MY_NR and i.alive:
                lowest_other = i.player_nr
                break

        find = self.is_there(self.combat.kill)
        if find != -1:
                pl_nr = self.commands[find][1][0]
                if fabs(self.players[pl_nr].v_x) == 0 and fabs(self.players[pl_nr].v_y) == 0:
                    self.commands[find][1] = [lowest_other, False]
                else:
                    self.commands[find][1] = [lowest_other, True]
        else:
            if fabs(self.players[lowest_other].v_x) == 0 and fabs(self.players[lowest_other].v_y) == 0:
                self.commands.append([self.combat.kill, [lowest_other, False]])
            else:
                self.commands.append([self.combat.kill, [lowest_other, True]])

    def algorithm(self):
        self.move_randomly(0.5)
        self.kill_lowest_hp()
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