__author__ = 'Kalmar'

import settings
from math import fabs, atan2, pi, copysign, sqrt
import random

DEBUG_COMBAT = False


class ActionFinished(Exception):
    function = None
    return_values = None

    def __init__(self, function, msg='Done', values=None):
        self.value = settings.color("[!] " + msg, "YELLOW")
        self.function = function
        self.return_values = values

    def __str__(self):
        return str(self.value)

    def oh_crap(self):
        print("Something went REALLY wrong")
        exit(0)


class Combat(object):
    """
    Algorithms compatible with gui and bots.
    You can keep here important data about battlefield: statistics, logins, points, velocities.
    It is handful to have it in one place.
    """

    players = None
    server = None
    movement_direction = None
    bunch_of_useful_stuff = None  # example

    def __init__(self, server):
        self.server = server

    def reload(self):
        for player in self.players:
            if player.shot:
                player.shoot()
        if self.players[settings.MY_NR].ammo < 10:
            self.players[settings.MY_NR].reloading = True
        if self.players[settings.MY_NR].ammo > 0.5 * settings.AMMO:
            if self.players[settings.MY_NR].reloading:
                self.players[settings.MY_NR].reloading = False
        for i in xrange(len(self.players)):
            self.players[i].counter += 1
            if self.players[i].ammo > 0:
                if self.players[i].counter > 1:
                    self.players[i].counter = 0
                    self.players[i].ammo += 0 if self.players[i].ammo >= settings.AMMO else 1
            else:
                if self.players[i].counter > 50:
                    self.players[i].counter = 0
                    self.players[i].ammo += 1

    def get_velocities(self):
        for i in xrange(len(self.players)):
            self.players[i].v_x = 1.0 * (self.players[i].x - self.players[i].old_x)
            self.players[i].v_y = 1.0 * (self.players[i].y - self.players[i].old_y)

    def update(self, players):
        self.players = players
        self.get_velocities()
        self.reload()

    def target_static_player(self, player_number):
        m_angle = self.players[settings.MY_NR].angle
        m_x, m_y = self.players[settings.MY_NR].x + settings.BLOCK_SIZE / 2.0, -(
            self.players[settings.MY_NR].y + settings.BLOCK_SIZE / 2.0)
        t_x, t_y = self.players[player_number].x + settings.BLOCK_SIZE / 2.0, - (
            self.players[player_number].y + settings.BLOCK_SIZE / 2.0)
        v = (t_x - m_x, t_y - m_y)
        tmp = atan2(v[1], v[0])
        v_ang = tmp if tmp >= 0 else tmp + 2 * pi
        ang_diff = v_ang - m_angle
        if fabs(ang_diff) <= (0.55 * settings.GUN_RESOLUTION):
            raise ActionFinished(self.target_static_player, "Player {} targeted".format(player_number))
        if ang_diff < 0:
            if ang_diff >= -pi:
                return self.server.rot("RIGHT")
            else:
                return self.server.rot("LEFT")
        if ang_diff > 0:
            if ang_diff < pi:
                return self.server.rot("LEFT")
            else:
                return self.server.rot("RIGHT")

    def kill(self, player_number):
        if player_number == settings.MY_NR:
            raise ActionFinished(self.kill, "Stop this suicidal thoughts!")
        if not self.players[player_number].alive:
            raise ActionFinished(self.kill, "Player {} killed".format(player_number))
        try:
            return self.target_static_player(player_number)
        except ActionFinished as e:
            if e.function == self.target_static_player:
                command = 0
                if DEBUG_COMBAT or settings.DEBUG:
                    print e
                if not self.players[settings.MY_NR].reloading:
                    command |= self.server.shoot()
                return command
            else:
                e.oh_crap()

    def near(self, val1, val2, epsilon):
        if (fabs(val1 - val2) < fabs(epsilon) or fabs(val1) > fabs(val2)) and copysign(1, val1) == copysign(1, val2):
            return True
        return False

    def move(self, direction, percent=1.0):
        number = settings.MY_NR
        speed = settings.BLOCK_SPEED
        dic = dict(LEFT=-speed, RIGHT=speed, UP=-speed, DOWN=speed)
        if direction.upper() == "LEFT" or direction.upper() == "RIGHT":
            if self.near(self.players[number].v_x, dic[direction.upper()] * percent,
                         settings.BLOCK_ACCELERATION) and self.near(self.players[number].v_y, 0,
                                                                    settings.BLOCK_ACCELERATION / 2):
                raise ActionFinished(self.move, "Moved {}".format(direction.lower()))
            else:
                return self.server.move(direction.upper())
        if direction.upper() == "UP" or direction.upper() == "DOWN":
            if self.near(self.players[number].v_y, dic[direction.upper()] * percent,
                         settings.BLOCK_ACCELERATION) and self.near(self.players[number].v_x, 0,
                                                                    settings.BLOCK_ACCELERATION / 2):
                raise ActionFinished(self.move, "Moved {}".format(direction.lower()))
            else:
                return self.server.move(direction.upper())






