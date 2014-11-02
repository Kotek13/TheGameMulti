from conversation import Conversation, Player
from time import sleep
from math import atan2, fabs, pi
import settings


class Simple(object):
    server = None
    players = []
    command = 0
    connected = False
    gui_killed = 0

    def __init__(self, server):
        self.server = server

    def targeted(self, nr):
        m_angle = self.players[settings.MY_NR].angle
        m_x, m_y = self.players[settings.MY_NR].x, -self.players[settings.MY_NR].y
        t_x, t_y = self.players[nr].x, -self.players[nr].y
        v = (t_x - m_x, t_y - m_y)
        tmp = atan2(v[1], v[0])
        v_ang = tmp if tmp >= 0 else tmp + 2*pi
        ang_diff = v_ang - m_angle
        if settings.DEBUG:
            print fabs(ang_diff), (1.0 * settings.BLOCK_SIZE/settings.MAP_SIZE)
        if fabs(ang_diff) <= (1.0 * settings.BLOCK_SIZE/settings.MAP_SIZE):
            return True,
        return False, ang_diff

    def target_player(self, server, nr):
        if not self.connected:
            self.players = server.get_players()
        if not self.players:
            return False
        target = self.targeted(nr)
        if target[0]:
            return True
        if target[1] > 0:
            self.command |= self.server.parse_command(("ROT_LEFT",))
        else:
            self.command |= self.server.parse_command(("ROT_RIGHT",))
        return False

    def kill_player(self, nr):
        while True:
            if not self.players[nr].alive:
                break
            self.command = 0
            if self.target_player(self.server, nr):
                self.command |= self.server.parse_command(("SHOOT",))
            self.server.send_commands(self.command)

    def killemall_static(self):
        self.server.hello()
        self.players = self.server.get_players()
        if settings.DEBUG:
            print self.players
        x = len(self.players)
        for i in xrange(x):
            if i != settings.MY_NR and self.players[i].alive:
                self.kill_player(i)
                sleep(4)
        return

    def gui_connect(self, players):
        self.players = players
        self.connected = True

    def gui_kill(self, nr):
        if nr == settings.MY_NR:
            raise UserWarning
        self.command = 0
        if not self.players[nr].alive:
            raise UserWarning
        target = self.target_player(self.server, nr)
        if target:
            self.command |= self.server.parse_command(("SHOOT",))
        return self.command

    def gui_killall(self):
        try:
            result = self.gui_kill(self.gui_killed)
            return result
        except UserWarning:
            self.gui_killed += 1
            if self.gui_killed >= len(self.players):
                self.gui_killed = 0
                raise UserWarning
            return self.server.parse_command(())
