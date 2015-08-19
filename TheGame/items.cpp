#include "stdafx.h"
#include "bullet.h"
#include "server.h"
// BULLET

void bullet::move()
{
	this->x = this->x + this->v_x;
	this->y = this->y + this->v_y;

	if (this->x >= MAP_SIZE - BULLET_SIZE || this->x <= 0)
		this->alive = false;
	if (this->y >= MAP_SIZE - BULLET_SIZE || this->y <= 0)
		this->alive = false;
}

void bullet::draw()
{
	if (this->alive)
		al_draw_filled_rectangle(this->x, this->y, this->x + BULLET_SIZE, this->y + BULLET_SIZE, this->player_color);
}

bullet::bullet(player_t * Player)
{
	this->alive = true;
	this->player_color = Player->color;
	this->owner_id = Player->login_hash;
	this->x = Player->x + BLOCK_SIZE / 2 + cos(Player->gun_alpha) * (BLOCK_SIZE / 2) * GUN_SIZE -1;
	this->y = Player->y + BLOCK_SIZE / 2 - sin(Player->gun_alpha) * (BLOCK_SIZE / 2) * GUN_SIZE -1;
	this->v_x = BULLET_SPEED * cos(Player->gun_alpha);
	this->v_y = -BULLET_SPEED * sin(Player->gun_alpha);
}

// PLAYER
bool states::get(choice c)
{
	size_t val = static_cast<size_t>(c);
	return ((choices >> c) & 1) == 1;
};

player::player()
{
	this->login_hash = 0;
	this->connected = 0;
	this->state = new struct states[1];
	this->dane = new struct sockaddr_in[1];
	this->color = al_map_rgb(255, 0, 0);

	ZeroMemory(this->dane, sizeof(struct sockaddr_in));
	spawn();

	this->x = 0;
	this->points = 0;
	this->y = 0;
}
void player::change_state()
{
	if (this->state->get(SHOOT))
		this->shots_left = 1;

	if (ammo) {
		if (++this->counter > 1)
		{
			this->counter = 0;
			this->ammo += (ammo >= MAX_AMMO) ? 0 : 1;
		}
	}	
	else {
		this->shots_left = 0;
		if (++this->counter > 50){
			this->counter = 0;
			this->ammo += 1;
		}
	}
	float v1 = this->v_x * this->v_x + this->v_y * this->v_y;

	if (this->state->get(ROT_LEFT))
		this->gun_alpha = this->gun_alpha + GUN_RESOLUTION;
	
	if (this->state->get(ROT_RIGHT))
		this->gun_alpha = (this->gun_alpha - GUN_RESOLUTION);

	if (this->state->get(MOVE_LEFT))
		this->v_x = this->v_x - BLOCK_ACCELERATION;
	
	if (this->state->get(MOVE_RIGHT))
		this->v_x = this->v_x + BLOCK_ACCELERATION;
	
	if (this->state->get(MOVE_UP))
		this->v_y = this->v_y - BLOCK_ACCELERATION;

	if (this->state->get(MOVE_DOWN))
		this->v_y = this->v_y + BLOCK_ACCELERATION;

	if (this->gun_alpha > 3.1415926 * 2)
		this->gun_alpha -= 3.1415926 * 2;
	if (this->gun_alpha < 0)
		this->gun_alpha += (3.1415926 * 2);

	float v2 = this->v_x * this->v_x + this->v_y * this->v_y;
	if (v2 > BLOCK_MAX_SPEED * BLOCK_MAX_SPEED)
	{
		this->v_x = sqrt(v1 / v2) * this->v_x;
		this->v_y = sqrt(v1 / v2) * this->v_y;
	}
	this->state->choices = 0;
}
void player::step()
{
	if (this->x + this->v_x > MAP_SIZE - BLOCK_SIZE)
		this->x = 0; // this->v_x = -this->v_x; //
	if (this->x + this->v_x < 0)
		this->x = MAP_SIZE - BLOCK_SIZE; // this->v_x = -this->v_x; //
	if (this->y + this->v_y > MAP_SIZE - BLOCK_SIZE)
		this->y = 0; // this->v_y = -this->v_y; //
	if (this->y + this->v_y < 0)
		this->y = MAP_SIZE - BLOCK_SIZE; //this->v_y = -this->v_y; // 

	this->x = this->x + this->v_x;
	this->y = this->y + this->v_y;
}
void player::move()
{
	if (this->alive)
	{
		this->shot = false;
		this->change_state();
		this->step();
	}		
}
bool player::is_in(float X, float Y)
{
	return (X >= this->x) && (X <= (this->x + BLOCK_SIZE)) && (Y >= this->y) && (Y <= (this->y + BLOCK_SIZE));
}
void player::spawn()
{
	this->alive = true;
	this->respawn = 0;
	this->shots_left = 0;
	this->v_x = 0;
	this->v_y = 0;
	this->ammo = MAX_AMMO;
	this->hp = HP;
	this->state->choices = 0;
	this->gun_alpha = 0;
	this->shot = false;
}
void player::draw()
{
	if (this->alive)
	{
		al_draw_filled_rectangle(this->x, this->y, this->x + BLOCK_SIZE, this->y + BLOCK_SIZE, this->color);
		al_draw_line(this->x + BLOCK_SIZE / 2, this->y + BLOCK_SIZE / 2, this->x + BLOCK_SIZE / 2 + cos(this->gun_alpha) * (BLOCK_SIZE / 2) * GUN_SIZE, this->y + BLOCK_SIZE / 2 - sin(this->gun_alpha) * (BLOCK_SIZE / 2) * GUN_SIZE, al_map_rgb(255, 255, 255), 2);
	}
	else
	{
		if (++this->respawn > RESPAWN_TIME * FPS)
			spawn();
	}
}
bool player::collision(player_t &P)
{
	return this->is_in(P.x, P.y) || this->is_in(P.x + BLOCK_SIZE, P.y) || this->is_in(P.x + BLOCK_SIZE, P.y + BLOCK_SIZE) || this->is_in(P.x, P.y + BLOCK_SIZE);
}
void player::shoot(list < bullet_t > *bullets, ALLEGRO_SAMPLE * shoot)
{
	if (this->shots_left > 0 && this->ammo > 0)
	{
			bullets->push_back(bullet(this));
			this->ammo--;
			this->shots_left--;
			this->shot = true;
			//al_play_sample(shoot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}
}