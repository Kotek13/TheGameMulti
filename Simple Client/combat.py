__author__ = 'Kalmar'

import settings
from math import fabs, atan2, pi
from conversation import Conversation


class Combat(object):
    """
    Algorithms compatible with gui and bots.
    You can keep here important data about battlefield: statistics, logins, points, velocities.
    It is handful to have it in one place.
    """

    players = None
    server = None
    bunch_of_useful_stuff = None #example

    def __init__(self, server):
        self.server = server

    def update(self, players):
        self.players = players

    def shoot(self):
        return self.server.parse_command(["SHOOT"])

    def rot(self, direction):
        if direction.upper() == "RIGHT":
            return self.server.parse_command(["ROT_RIGHT"])
        if direction.upper() == "LEFT":
            return self.server.parse_command(["ROT_LEFT"])
        return 0

    def angle_difference(self, player_number):
        m_angle = self.players[settings.MY_NR].angle
        m_x, m_y = self.players[settings.MY_NR].x, -self.players[settings.MY_NR].y
        t_x, t_y = self.players[player_number].x, -self.players[player_number].y
        v = (t_x - m_x, t_y - m_y)
        tmp = atan2(v[1], v[0])
        v_ang = tmp if tmp >= 0 else tmp + 2 * pi
        return v_ang - m_angle

    def target_static_player(self, player_number):
        ang_diff = self.angle_difference(player_number)
        if fabs(ang_diff) <= (0.55 * settings.GUN_RESOLUTION):
            return True,
        if ang_diff < 0:
            if ang_diff >= -pi:
                return False, self.rot("RIGHT")
            else:
                return False, self.rot("LEFT")
        if ang_diff > 0:
            if ang_diff < pi:
                return False, self.rot("LEFT")
            else:
                return False, self.rot("RIGHT")

    def kill(self, player_number):
        if player_number == settings.MY_NR:
            print settings.color("Stop this suicidal thoughts!", "CYAN")
            raise UserWarning
        if not self.players[player_number].alive:
            raise UserWarning
        target = self.target_static_player(player_number)
        if target[0]:
            if not self.players[settings.MY_NR].reloading:
                return self.shoot()
            return 0
        return target[1]


