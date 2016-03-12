#include "stdafx.h"
#include "bullet.h"
#include "server.h"
// BULLET

void bullet::move()
{
	this->x = this->x + this->v_x;
	this->y = this->y + this->v_y;

	if (this->x >= game->settings.map_size - game->settings.bullet_size || this->x <= 0)
		this->alive = false;
	if (this->y >= game->settings.map_size - game->settings.bullet_size || this->y <= 0)
		this->alive = false;
}

void bullet::draw()
{
	if (this->alive)
		al_draw_filled_rectangle(this->x, this->y, this->x + game->settings.bullet_size, this->y + game->settings.bullet_size, this->player_color);
}

bullet::bullet(player_t * Player)
{
	this->alive = true;
	this->player_color = Player->color;
	this->owner_id = Player->login_hash;
	this->x = Player->x + game->settings.block_size / 2 + cos(Player->gun_alpha) * (game->settings.block_size / 2) * game->settings.gun_size -1;
	this->y = Player->y + game->settings.block_size / 2 - sin(Player->gun_alpha) * (game->settings.block_size / 2) * game->settings.gun_size -1;
	this->v_x = game->settings.bullet_speed * cos(Player->gun_alpha);
	this->v_y = -game->settings.bullet_speed * sin(Player->gun_alpha);
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
			this->ammo += (ammo >= game->settings.max_ammo) ? 0 : 1;
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
		this->gun_alpha = this->gun_alpha + game->settings.gun_resolution;
	
	if (this->state->get(ROT_RIGHT))
		this->gun_alpha = (this->gun_alpha - game->settings.gun_resolution);

	if (this->state->get(MOVE_LEFT))
		this->v_x = this->v_x - game->settings.block_acceleration;
	
	if (this->state->get(MOVE_RIGHT))
		this->v_x = this->v_x + game->settings.block_acceleration;
	
	if (this->state->get(MOVE_UP))
		this->v_y = this->v_y - game->settings.block_acceleration;

	if (this->state->get(MOVE_DOWN))
		this->v_y = this->v_y + game->settings.block_acceleration;

	if (this->gun_alpha > 3.1415926 * 2)
		this->gun_alpha -= 3.1415926 * 2;
	if (this->gun_alpha < 0)
		this->gun_alpha += (3.1415926 * 2);

	float v2 = this->v_x * this->v_x + this->v_y * this->v_y;
	if (v2 > game->settings.block_max_speed * game->settings.block_max_speed)
	{
		this->v_x = sqrt(v1 / v2) * this->v_x;
		this->v_y = sqrt(v1 / v2) * this->v_y;
	}
	this->state->choices = 0;
}
void player::step()
{
	if (this->x + this->v_x > game->settings.map_size - game->settings.block_size)
		this->x = 0; // this->v_x = -this->v_x; //
	if (this->x + this->v_x < 0)
		this->x = game->settings.map_size - game->settings.block_size; // this->v_x = -this->v_x; //
	if (this->y + this->v_y > game->settings.map_size - game->settings.block_size)
		this->y = 0; // this->v_y = -this->v_y; //
	if (this->y + this->v_y < 0)
		this->y = game->settings.map_size - game->settings.block_size; //this->v_y = -this->v_y; // 

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
	return (X >= this->x) && (X <= (this->x + game->settings.block_size)) && (Y >= this->y) && (Y <= (this->y + game->settings.block_size));
}
void player::spawn()
{
	this->alive = true;
	this->respawn = 0;
	this->shots_left = 0;
	this->v_x = 0;
	this->v_y = 0;
	this->ammo = game->settings.max_ammo;
	this->hp = game->settings.hp;
	this->state->choices = 0;
	this->gun_alpha = 0;
	this->shot = false;
}
void player::draw()
{
	if (this->alive)
	{
		al_draw_filled_rectangle(this->x, this->y, this->x + game->settings.block_size, this->y + game->settings.block_size, this->color);
		al_draw_line(this->x + game->settings.block_size / 2, this->y + game->settings.block_size / 2, this->x + game->settings.block_size / 2 + cos(this->gun_alpha) * (game->settings.block_size / 2) * game->settings.gun_size, this->y + game->settings.block_size / 2 - sin(this->gun_alpha) * (game->settings.block_size / 2) * game->settings.gun_size, al_map_rgb(255, 255, 255), 2);
	}
	else
	{
		if (++this->respawn > game->settings.respawn_time * game->settings.fps)
			spawn();
	}
}
bool player::collision(player_t &P)
{
	return this->is_in(P.x, P.y) || this->is_in(P.x + game->settings.block_size, P.y) || this->is_in(P.x + game->settings.block_size, P.y + game->settings.block_size) || this->is_in(P.x, P.y + game->settings.block_size);
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