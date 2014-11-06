__author__ = 'Kalmar'

"""
Pure GUI client to play over net.
"""

from math import cos, sin
import settings
import pygame
from conversation import Conversation


class Gui(object):
    server = None
    players = []
    bullets = []
    display = None
    counter = 0
    commands = {pygame.K_RIGHT: 'MOVE_RIGHT', pygame.K_LEFT: 'MOVE_LEFT', pygame.K_UP: 'MOVE_UP', pygame.K_DOWN: 'MOVE_DOWN', pygame.K_PERIOD: 'ROT_RIGHT', pygame.K_COMMA: 'ROT_LEFT', pygame.K_SLASH: 'SHOOT'}

    def __init__(self, server):
        self.server = server
        pygame.init()
        self.display = pygame.display.set_mode((int(settings.MAP_SIZE), int(settings.MAP_SIZE+ settings.FOOTER_SIZE)), 0, 24)
        pygame.display.set_caption("The Game client - {} [ADD LOGIN SERVER]".format(settings.IP))

    def draw_footer(self):
        for i in xrange(len(self.players)):
            if self.players[i].alive:
                if self.players[i].player_nr == settings.MY_NR:
                    cl = pygame.Color(255, 0, 0)
                else:
                    cl = pygame.Color(0, 0, 255)
                pygame.draw.rect(self.display, cl, (
                settings.MAP_SIZE / len(self.players) * i, settings.MAP_SIZE + 2, settings.MAP_SIZE / len(self.players) * self.players[i].hp / settings.HP, settings.FOOTER_SIZE - 2))
                pygame.draw.line(self.display, pygame.Color(255, 255, 255), (
                settings.MAP_SIZE / len(self.players) * i, settings.MAP_SIZE), (
                                 settings.MAP_SIZE / len(self.players) * i + settings.MAP_SIZE / len(self.players) * self.players[i].ammo / settings.AMMO, settings.MAP_SIZE), 2)
        for i in range(1, len(self.players)):
            pygame.draw.rect(self.display, pygame.Color(0, 0, 0), (
            settings.MAP_SIZE / len(self.players) * i, settings.MAP_SIZE, 1, settings.FOOTER_SIZE))

    def draw(self):
        self.display.fill(pygame.Color(0, 0, 0))
        for i in self.bullets:
            if not i.alive:
                self.bullets.pop(self.bullets.index(i))
        for i in xrange(len(self.players)):
            if not self.players[i].alive:
                continue
            if self.players[i].player_nr == settings.MY_NR:
                cl = pygame.Color(255, 0, 0)
            else:
                cl = pygame.Color(0, 0, 255)
            thick = int(settings.THICKNESS)
            b_size = int(settings.BLOCK_SIZE)
            if self.players[i].full:
                self.players[i].ammo = settings.AMMO
            player = self.players[i]
            pygame.draw.rect(self.display, cl, (int(player.x), int(player.y), b_size, b_size), thick)
            pygame.draw.line(self.display, pygame.Color(255, 255, 255), (player.x + b_size / 2, player.y + b_size / 2), (int(player.x + b_size / 2 + cos(player.angle) * (b_size / 2) * 1.9), int(player.y + b_size / 2 - sin(player.angle) * (b_size / 2) * 1.9)), 2)
        for i in xrange(len(self.bullets)):
            if self.bullets[i].owner_nr == settings.MY_NR:
                cl = pygame.Color(255, 0, 0)
            else:
                cl = pygame.Color(0, 0, 255)
            bullet = self.bullets[i]
            pygame.draw.rect(self.display, cl, (int(bullet.x), int(bullet.y), settings.BULLET_SIZE, settings.BULLET_SIZE))
        self.draw_footer()
        pygame.display.update()

    def move_bullets(self):
        for i in xrange(len(self.bullets)):
            self.bullets[i].move()
            if self.bullets[i].x >= settings.MAP_SIZE - settings.BULLET_SIZE or self.bullets[i].x <= 0:
                self.bullets[i].alive = False
            if self.bullets[i].y >= settings.MAP_SIZE - settings.BULLET_SIZE or self.bullets[i].y <= 0:
                self.bullets[i].alive = False

    def shoot(self):
        for i in xrange(len(self.players)):
            player = self.players[i]
            if player.shot:
                if player.ammo > 0:
                    self.bullets.append(player.shoot())

    def add_keyboard(self):
        move = []
        keys = pygame.key.get_pressed()
        if keys[pygame.K_ESCAPE]:
            exit()
        for i in self.commands:
            if keys[i]:
                move.append(self.commands[i])
        self.server.send_command(move)

    def check_hits(self):
        for i in xrange(len(self.bullets)):
            for j in xrange(len(self.players)):
                if self.players[j].hit(self.bullets[i]):
                    self.bullets[i].alive = False
                    break

    def reload(self):
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

    def run(self):
        self.server.hello()
        while True:
            pygame.event.pump()
            self.players = self.server.get_players(self.players)
            if self.players:
                self.move_bullets()
                self.reload()
                self.shoot()
                self.check_hits()
                self.draw()
                self.add_keyboard()


if __name__ == '__main__':
    import sys
    settings.IP = sys.argv[1]
    settings.MY_NR = int(sys.argv[2], 10)
    settings.PIN = int(sys.argv[3], 16)
    Gui(Conversation()).run()
