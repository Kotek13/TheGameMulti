#ifndef BULLET
#define BULLET

#include "stdafx.h"
#include "player.h"

using namespace std;

struct bullet
{
	float x, v_x;
	float y, v_y;
	bool alive;
	size_t owner_id;
	ALLEGRO_COLOR player_color;

	bullet(player * Player);
	bullet() {}

	void move();
	void draw();
};

#endif