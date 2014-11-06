#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/timer.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <vector>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define BLOCK_SIZE 15
#define MAP_SIZE 700
#define FPS 60
#define BLOCK_ACCELERATION ((float)BLOCK_MAX_SPEED/FPS*2)
#define BLOCK_MAX_SPEED (MAP_SIZE/FPS)
#define BULLET_SIZE 2
#define GUN_SIZE 1.9
#define FOOTER_SIZE 20
#define BULLET_SPEED (1.5*BLOCK_MAX_SPEED)
#define THICKNESS 2
#define RESPAWN_TIME 2
#define GUN_RESOLUTION ((float)BLOCK_SIZE/MAP_SIZE*2)
#define MAX_AMMO 200
#define HP 200
#define COLOR_WHITE al_map_rgb(255, 255, 255)
#define COLOR_BLACK al_map_rgb(0, 0, 0)