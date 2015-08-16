#ifndef _GAME_H
#define _GAME_H

#include "stdafx.h"

struct game_t
{
	ALLEGRO_KEYBOARD_STATE klawiatura;
	ALLEGRO_DISPLAY * okno;
	ALLEGRO_FONT * font, *foot_font, *menu_font;
	ALLEGRO_TIMER * timer;
	ALLEGRO_EVENT_QUEUE * queue;
	ALLEGRO_EVENT event;
	ALLEGRO_SAMPLE *theme, *shoot;

	USHORT port;
	int interrupted;
	sqlite3 * db;

	vector < player_t > players;
	list < bullet_t > bullets;

};

extern game_t * game;

#endif