__author__ = 'Kalmar'
from hybrid import Hybrid
from bot import Bot
from conversation import Conversation

server = Conversation()


def play():
    global server
    x = Hybrid(server)
    x.run()

play()
