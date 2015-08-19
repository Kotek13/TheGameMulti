import sqlite3
from time import sleep
import HTML, datetime


class Player(object):
	color = 0
	id = 0
	points = 0
	login = ""


class PointManager(object):
	def __init__(self, db_path = "TheGame.db", html_path = "index.html"):
		self.html = html_path
		self.connect_db(db_path)
		self.players = []
		self.get_players()

	def connect_db(self, db_path):
		self.cursor = sqlite3.connect(db_path).cursor()

	def get_players(self):
		self.players = []
		for row in self.cursor.execute("SELECT * from USERS ORDER BY ID"):
			pl = Player()
			pl.id = row[0]
			pl.login = row[1]
			pl.color = row[2]
			pl.points = row[3]
			self.players.append(pl)

	def generate_table(self):
		t = HTML.Table(header_row=['ID', 'LOGIN', 'POINTS', 'COLOR'])
		for pl in sorted(self.players, key=lambda x: -x.points):
			t.rows.append([pl.id, pl.login, pl.points, HTML.TableCell(" ", bgcolor="#{}".format(pl.color))])
		buf = "<html><head>"
		buf +=  '<meta http-equiv="refresh" content="5">'
		buf += '<title>TheGame - Day %d </title>' % (datetime.date.today().day - 17)
		buf += '</head>' 
		buf += str(t).replace('&nbsp;', '0')
		buf += '</html>'
		self.dump_html(buf)

	def dump_html(self, string):
		f = open(self.html, "w")
		f.write(string)
		f.close()


pm = PointManager()

try:
	while True:
		pm.get_players()
		pm.generate_table()
		sleep(5.5)
except KeyboardInterrupt:
	pass
