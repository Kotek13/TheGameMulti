from conversation import Conversation, Player
from time import sleep
from math import atan2, fabs, pi
import settings


class Simple(object):
    server = None
    players = [Player]

    def __init__(self):
        self.server = Conversation()
        self.server.hello()

    def target_player(self, server, nr):
        while True:
            self.players = server.get_players()
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
                return True
            if ang_diff > 0:
                server.send_commands(("ROT_LEFT",))
            else:
                server.send_commands(("ROT_RIGHT",))

    def kill_player(self, nr):
        while True:
            self.target_player(self.server, nr)
            if not self.players[nr].alive:
                break
            self.server.send_commands(("SHOOT",))

    def killemall_static(self):
        self.players = self.server.get_players()
        if settings.DEBUG:
            print self.players
        x = len(self.players)
        for i in xrange(x):
            if i != settings.MY_NR and self.players[i].alive:
                self.kill_player(i)
                sleep(4)
        return