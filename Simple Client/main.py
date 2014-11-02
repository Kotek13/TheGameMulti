__author__ = 'Kalmar'
from gui import Gui
from simple import Simple
from conversation import Conversation

server = Conversation()


def play():
    global server
    x = Gui(server)
    x.run()


def run_bot():
    global server
    bot = Simple(server)
    while True:
        bot.killemall_static()

play()
