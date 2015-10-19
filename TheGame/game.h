#ifndef _GAME_H
#define _GAME_H

#include "stdafx.h"

struct game_t
{
	ALLEGRO_KEYBOARD_STATE keyboard;
	ALLEGRO_DISPLAY * window;
	ALLEGRO_FONT * font, *foot_font, *menu_font;
	ALLEGRO_TIMER * timer;
	ALLEGRO_EVENT_QUEUE * queue;
	ALLEGRO_EVENT event;
	ALLEGRO_SAMPLE *theme, *shoot;

	USHORT port;
	int interrupted;
	int footer_rows;
	sqlite3 * db;
	unsigned int counter;

	TG_SOCKET points_server;

	vector < player_t > players;
	list < bullet_t > bullets;

	settings_t settings;

};

extern game_t * game;

void game_loop(void);
int setup_players();
void setup_window();
int connect_point_server();

#endif