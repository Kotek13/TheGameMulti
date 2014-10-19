__author__ = 'Kalmar'
from Player import Player
from Simple import Simple
from time import sleep

settings = {"IP": "192.168.1.10",
            "PORT": 8081,
            "ID": 0xbc5c2cdc,
            "NR": 1,
            "FPS": 60,
            "MAP_SIZE": 600.0,
            "BLOCK_SIZE": 15.0,
            "BULLET_SIZE": 2.0,
            "THICKNESS": 2.0,
            "MAX_AMMO": 200,
            "HP": 200,
            }


def play():
    x = Player(settings)
    x.run()


def run_bot():
    bot = Simple(settings)
    while True:
        bot.killemall_static()

play()
