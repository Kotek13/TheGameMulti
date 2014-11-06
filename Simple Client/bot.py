import settings
from combat import Combat, ActionFinished
from conversation import Conversation

DEBUG_BOT = False
TYPE = 'static'


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
        for i in self.commands:
            if args:
                if [command, args] == i:
                    return True
            else:
                if i[0] == command:
                    return True
        return False

    def algorithm(self):
        global TYPE
        if not self.is_there(self.combat.random_movement, [0.5]):
            self.commands.append([self.combat.random_movement, [0.5]])
        if not self.is_there(self.combat.kill):
            for i in sorted(self.players, key=lambda x: x.hp):
                if i.alive and i.player_nr != settings.MY_NR:
                    self.commands.append([self.combat.kill, [i.player_nr, TYPE]])
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
    TYPE = sys.argv[4]
    Bot(Conversation()).run()