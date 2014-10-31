__author__ = 'Kalmar'
import pygame
from math import cos, sin
from conversation import Conversation, Player
import settings


class Gui(object):
    server = None
    players = [Player]
    display = None
    commands = {pygame.K_RIGHT: 'MOVE_RIGHT', pygame.K_LEFT: 'MOVE_LEFT', pygame.K_UP: 'MOVE_UP', pygame.K_DOWN: 'MOVE_DOWN', pygame.K_PERIOD: 'ROT_RIGHT', pygame.K_COMMA: 'ROT_LEFT', pygame.K_SLASH: 'SHOOT'}

    def __init__(self):
        self.server = Conversation()
        pygame.init()
        self.display = pygame.display.set_mode((int(settings.MAP_SIZE), int(settings.MAP_SIZE)), 0, 24)
        pygame.display.set_caption("The Game client - {}".format(settings.IP))

    def draw(self):
        self.display.fill(pygame.Color(0, 0, 0))
        for i in xrange(len(self.players)):
            if not self.players[i].alive:
                continue
            if self.players[i].player_nr == settings.MY_NR:
                cl = pygame.Color(255, 0, 0)
            else:
                cl = pygame.Color(255, 255, 255)
            thick = int(settings.THICKNESS)
            b_size = int(settings.BLOCK_SIZE)
            player = self.players[i]
            pygame.draw.rect(self.display, cl, (player.x, player.y, b_size, b_size), thick)
            pygame.draw.line(self.display, pygame.Color(255, 255, 255), (player.x + b_size / 2, player.y + b_size / 2), (int(player.x + b_size / 2 + cos(player.angle) * (b_size / 2) * 1.9), int(player.y + b_size / 2 - sin(player.angle) * (b_size / 2) * 1.9)), 2)
        pygame.display.update()

    def send_keyboard(self):
        move = []
        keys = pygame.key.get_pressed()
        if keys[pygame.K_ESCAPE]:
            exit()
        for i in self.commands:
            if keys[i]:
                move.append(self.commands[i])
        self.server.send_commands(move)

    def run(self):
        self.server.hello()
        while True:
            pygame.event.pump()
            self.players = self.server.get_players()
            if self.players:
                self.draw()
                self.send_keyboard()




