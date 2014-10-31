__author__ = 'Kalmar'
from gui import Gui
from simple import Simple


def play():
    x = Gui()
    x.run()


def run_bot():
    bot = Simple()
    while True:
        bot.killemall_static()

play()
