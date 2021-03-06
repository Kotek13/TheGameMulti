#ifndef _BULLET_H
#define _BULLET_H

#include "stdafx.h"

using namespace std;

struct bullet
{
	float x, v_x;
	float y, v_y;
	bool alive;
	size_t owner_id;
	ALLEGRO_COLOR player_color;

	bullet(player_t * Player);
	bullet() {}

	void move();
	void draw();
};

#endif