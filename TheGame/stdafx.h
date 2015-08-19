#ifndef _STDAFX_H
#define _STDAFX_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/timer.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <csignal>
#include <vector>
#include <list>

#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cinttypes>
#include <sqlite3.h>

typedef struct bullet bullet_t;
typedef struct player player_t;

#include "bullet.h"
#include "player.h"
#include "server.h"
#include "game.h"

#define BLOCK_SIZE 15
#define MAP_SIZE 700
#define FPS 15
#define BLOCK_ACCELERATION ((float)BLOCK_MAX_SPEED/70*2)
#define BLOCK_MAX_SPEED ((float)MAP_SIZE/70)
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

#ifdef WIN32
#include <Windows.h>
typedef HANDLE HTHREAD;
#else
typedef pthread HTHREAD;
#define ZeroMemory((buf),(nb)) memset((buf), 0, (nb))
#include <pthread>
#endif

uint32_t crc32(uint32_t crc, const void *buf, size_t size);

#endif