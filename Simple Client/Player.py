__author__ = 'Kalmar'
import pygame
from math import cos, sin
from Conversation import Conversation


class Player(object):
    server = None
    board = None
    display = None
    settings = None
    commands = {pygame.K_RIGHT: 'MOVE_RIGHT', pygame.K_LEFT: 'MOVE_LEFT', pygame.K_UP: 'MOVE_UP', pygame.K_DOWN: 'MOVE_DOWN', pygame.K_PERIOD: 'ROT_RIGHT', pygame.K_COMMA: 'ROT_LEFT', pygame.K_SLASH: 'SHOOT'}

    def __init__(self, settings):
        self.server = Conversation(settings)
        self.settings = settings
        pygame.init()
        self.display = pygame.display.set_mode((int(self.settings['MAP_SIZE']), int(self.settings['MAP_SIZE'])), 0, 24)
        pygame.display.set_caption("The Game client - {}".format(self.settings['IP']))

    def draw(self):
        self.display.fill(pygame.Color(0, 0, 0))
        for i in xrange(len(self.board)):
            if not self.board[i]['alive']:
                continue
            if i == self.server.player_number:
                cl = pygame.Color(255, 0, 0)
            else:
                cl = pygame.Color(255, 255, 255)
            thick = int(self.settings['THICKNESS'])
            b_size = int(self.settings['BLOCK_SIZE'])
            player = self.board[i]
            pygame.draw.rect(self.display, cl, (player['x'], player['y'], b_size, b_size), thick)
            pygame.draw.line(self.display, pygame.Color(255, 255, 255), (player['x'] + b_size / 2, player['y'] + b_size / 2), (int(player['x'] + b_size / 2 + cos(player['angle']) * (b_size / 2) * 1.9), int(player['y'] + b_size / 2 - sin(player['angle']) * (b_size / 2) * 1.9)), 2)
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
            self.board = self.server.get_board()
            if self.board:
                self.draw()
                self.send_keyboard()




