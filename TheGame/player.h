#ifndef _PLAYER_H
#define _PLAYER_H

#include "stdafx.h"
#include "bullet.h"

using namespace std;

struct bullet;

enum choice { MOVE_RIGHT = 0, MOVE_LEFT = 1, MOVE_UP = 2, MOVE_DOWN = 3, ROT_RIGHT = 4, ROT_LEFT = 5, SHOOT = 6 };

struct states
{
	unsigned char choices;
	bool get(choice c);
};

struct player
{
	float x, v_x;
	float y, v_y;
	float gun_alpha;
	int ammo;
	int hp;
	size_t shots_left;
	size_t points;
	size_t respawn;
	size_t counter;
	size_t login_hash;
	struct sockaddr_in * dane;
	struct states * state;
	ALLEGRO_COLOR color;
	bool connected;
	bool alive;	
	bool shot;
	
	player();
	bool collision(player &P);
	bool is_in(float X, float Y);
	void draw();
	void spawn();
	void change_state();
	void step();
	void move();
	void shoot(list < bullet > &bullets, ALLEGRO_SAMPLE * shoot);
};
extern vector < player > players;
#endif