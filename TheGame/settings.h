#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "stdafx.h"

struct settings_t
{
	int block_size = 15;
	int map_size = 700;
	int fps = 60;
	float block_max_speed = ((float)map_size/70);
	float block_acceleration = ((float)block_max_speed/70*2);
	float bullet_size = 2;
	float gun_size = 1.9;
	float footer_size = 20;
	float bullet_speed = (1.5*block_max_speed);
	float thickness = 2;
	float respawn_time = 2;
	float gun_resolution = ((float)block_size/map_size*2);
	int max_ammo = 200;
	int hp = 200;
};

#endif