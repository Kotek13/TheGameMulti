__author__ = 'Kalmar'
from Player import Player
from Simple import Simple

settings = {"IP": "192.168.1.10",
            "PORT": 8081,
            "ID": 0x00dd9e31,
            "NR": 0,
            "FPS": 60,
            "MAP_SIZE": 600.0,
            "BLOCK_SIZE": 10.0,
            "BULLET_SPEED": 3}


def play():
    x = Player(settings)
    x.run()


def run_bot():
    bot = Simple(settings)
    bot.killemall_static()