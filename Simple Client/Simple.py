from Conversation import Conversation
from time import sleep
from math import atan2, fabs, pi


class Simple(object):
    debug = False
    server = None
    settings = {}
    glob_board = {}

    def __init__(self, settings):
        self.settings = settings
        self.server = Conversation(settings)
        self.server.hello()

    def target_player(self, server, nr):
        while True:
            self.glob_board = server.get_state()
            m_angle = self.glob_board[server.nr]["angle"]
            m_x, m_y = self.glob_board[server.nr]['x'], -self.glob_board[server.nr]['y']
            t_x, t_y = self.glob_board[nr]['x'], -self.glob_board[nr]['y']
            v = (t_x - m_x, t_y - m_y)
            tmp = atan2(v[1], v[0])
            v_ang = tmp if tmp >= 0 else tmp + 2*pi
            ang_diff = v_ang - m_angle
            if self.debug:
                print fabs(ang_diff), (self.settings["BLOCK_SIZE"]/self.settings["MAP_SIZE"])
            if fabs(ang_diff) <= (self.settings["BLOCK_SIZE"]/self.settings["MAP_SIZE"]):
                return True
            if ang_diff > 0:
                server.send_commands(("ROT_LEFT",))
            else:
                server.send_commands(("ROT_RIGHT",))

    def kill_player(self, nr):
        while True:
            self.target_player(self.server, nr)
            if not self.glob_board[nr]["alive"]:
                break
            self.server.send_commands(("SHOOT",))

    def killemall_static(self):
        self.glob_board = self.server.get_state()
        if self.debug:
            print self.glob_board
        x = len(self.glob_board)
        for i in xrange(x):
            if i != self.server.nr and self.glob_board[i]["alive"]:
                self.kill_player(i)
                sleep(4)
        return