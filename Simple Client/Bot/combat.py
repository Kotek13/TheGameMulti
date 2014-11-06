__author__ = 'Kalmar'

from math import fabs, atan2, pi, copysign, sqrt
import random
import settings

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
        #print self.players[settings.MY_NR].ammo
        if self.players[settings.MY_NR].ammo < 20:
            self.players[settings.MY_NR].reloading = True
        if self.players[settings.MY_NR].ammo > 0.5 * settings.AMMO:
            if self.players[settings.MY_NR].reloading:
                self.players[settings.MY_NR].reloading = False
        for i in xrange(len(self.players)):
            self.players[i].counter += 1
            if self.players[i].full:
                self.players[i].ammo = settings.AMMO
            if self.players[i].ammo > 0:
                if self.players[i].counter > 1:
                    self.players[i].counter = 0
                    self.players[i].ammo += 0 if self.players[i].ammo >= settings.AMMO else 1
                    if self.players[i].ammo >= settings.AMMO:
                        self.players[i].full = True
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
        m_x, m_y = self.players[settings.MY_NR].x, -self.players[settings.MY_NR].y
        t_x, t_y = self.players[player_number].x, -self.players[player_number].y
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

    def target_dynamic_player(self, player_number):
        # TODO: Take length of the gun and bouncing into consideration
        m_angle = self.players[settings.MY_NR].angle
        v_x, v_y = self.players[player_number].v_x, -self.players[player_number].v_y
        dx, dy = self.players[player_number].x - self.players[settings.MY_NR].x, - self.players[player_number].y + \
                 self.players[settings.MY_NR].y
        u = settings.BULLET_SPEED
        a, b, c = u * dx, u * dy, dx * v_y - dy * v_x
        t_angle = 2 * atan2(sqrt(a ** 2 + b ** 2 + c ** 2) - a, b - c)
        ang_diff = t_angle - m_angle if t_angle >= 0 else t_angle + 2 * pi - m_angle

        command = 0

        if ang_diff < 0:
            if ang_diff >= -pi:
                command = self.server.rot("RIGHT")
            else:
                command = self.server.rot("LEFT")
        if ang_diff > 0:
            if ang_diff < pi:
                command = self.server.rot("LEFT")
            else:
                command = self.server.rot("RIGHT")

        if fabs(ang_diff) <= (6 * settings.GUN_RESOLUTION):
            raise ActionFinished(self.target_dynamic_player, "Moving player {} almost targeted".format(player_number), [command])
        if fabs(ang_diff) <= 0.55 * settings.GUN_RESOLUTION:
            raise ActionFinished(self.target_dynamic_player, "Moving player {} targeted".format(player_number))
        return command

    def kill(self, player_number, dynamic=True):
        global DEBUG_COMBAT
        if dynamic:
            function = self.target_dynamic_player
        else:
            function = self.target_static_player

        if player_number == settings.MY_NR:
            raise ActionFinished(self.kill, "Stop this suicidal thoughts!")
        if not self.players[player_number].alive:
            raise ActionFinished(self.kill, "Player {} killed".format(player_number))
        try:
            return function(player_number)
        except ActionFinished as e:
            if e.function == function:
                command = 0
                if DEBUG_COMBAT or settings.DEBUG:
                    print(e)
                if e.return_values:
                    for i in e.return_values:
                        command |= i
                if not self.players[settings.MY_NR].reloading:
                    command |= self.server.shoot()
                return command
            else:
                e.oh_crap()

    def near(self, val1, val2, epsilon):
        # print val1, val2
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

    def random_movement(self, percent=1.0):
        if not self.movement_direction:
            options = ["LEFT", "RIGHT", "UP", "DOWN"]
            self.movement_direction = options[random.randint(0, 3)]
        try:
            return self.move(self.movement_direction, percent)
        except ActionFinished as e:
            if e.function == self.move:
                if DEBUG_COMBAT or settings.DEBUG:
                    print e
                self.movement_direction = None
                raise ActionFinished(self.random_movement, "Changed direction randomly")
            else:
                e.oh_crap()






