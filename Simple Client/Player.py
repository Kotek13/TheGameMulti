__author__ = 'Kalmar'
import pygame


class Player(object):
    settings = None
    commands = {pygame.K_RIGHT: 0, pygame.K_LEFT: 1, pygame.K_UP: 2, pygame.K_DOWN: 3, pygame.K_PERIOD: 4, pygame.K_COMMA: 5, pygame.K_SLASH: 6}

    def __init__(self, settings):
        self.settings = settings

    def run(self):
        pass

