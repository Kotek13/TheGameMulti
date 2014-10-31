__author__ = 'Kalmar'
from Player import Player
from Simple import Simple

settings = {"IP": "127.0.0.1",
            "PORT": 8081,
            "ID": 0x00dd9e31,
            "NR": 0,
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
