#include "stdafx.h"

#ifdef WIN32
#include <Windows.h>
typedef HANDLE HTHREAD;
#else
typedef pthread HTHREAD;
#define ZeroMemory((buf),(nb)) memset((buf), 0, (nb))
#include <pthread>
#endif

game_t * game = new game_t;

using namespace std;

void Message(const char * a)
{
	MessageBox(NULL, a, "Error", MB_OK);
}

HTHREAD StartThread(void(*function)(void))
{
#ifdef WIN32
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, game, 0, NULL);
#else
	pthread thread;
	pthread_create(&thread, 0, function, game);
	return;
#endif
}

void WaitForThread(HTHREAD thread)
{
#ifdef WIN32
	WaitForSingleObject(thread, INFINITE);
#else
	pthread_join(thread, NULL);
#endif
}

void check_collisions()
{
	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		for (vector<player_t>::iterator j = i + 1; j < game->players.end(); j++)
		{
			if (i->collision(*j))
			{
				float v_x_1 = i->v_x;
				float v_x_2 = j->v_x;
				float v_y_1 = i->v_y;
				float v_y_2 = j->v_y;
				
				i->v_x = v_x_2;
				i->v_y = v_y_2;
				j->v_x = v_x_1;
				j->v_y = v_y_1;

				i->step();
				j->step();
			}
		}
	}
}
void check_hits()
{

	for (auto i = game->players.begin(); i != game->players.end(); i++)
	{
		for (auto j = game->bullets.begin(); j != game->bullets.end(); j++)
		{

			if (i->is_in(j->x, j->y) && j->alive && i->alive)
			{
				vector<player_t>::iterator owner = find(j->owner_id);
				if (owner == game->players.end())
					return;
				bool same = false;
				if(i->login_hash == j->owner_id)
					same = true;
				if (!same)
					owner->points++;
				if (--i->hp <= 0 && !same && i->alive)
				{
					owner->points += 50;
					i->alive = false;
				}
					
				game->bullets.erase(j);
			}
		}
	}
}
void draw_footer()
{
	vector<player_t>::iterator beg = game->players.begin();

	for (vector<player_t>::iterator i = beg; i < game->players.end(); i++)
	{
		int col = (i - beg) % 4;
		int row = (i - beg) / 4;

		// hp bar
		if (i->alive)
			al_draw_filled_rectangle(MAP_SIZE / 4 * col, MAP_SIZE + 2 + row * FOOTER_SIZE, MAP_SIZE / 4 * col + MAP_SIZE / 4 * i->hp / HP, MAP_SIZE + FOOTER_SIZE * (row + 1), i->color);

		// ammo bar
		al_draw_line(MAP_SIZE / 4 * col, MAP_SIZE + 1 + FOOTER_SIZE * row, MAP_SIZE / 4 * col + MAP_SIZE / 4 * i->ammo / MAX_AMMO, MAP_SIZE + 1 + FOOTER_SIZE * row, al_map_rgb(255, 255, 255), 2);

		// login
		char * buffer = new char[100];
		sprintf_s(buffer, 100, "[%c] %s [%d]", i->connected ? '+' : '-', i->login.c_str(), i->points);
		
		al_draw_text(game->foot_font, COLOR_WHITE, MAP_SIZE / 4 * col + MAP_SIZE / 4 / 2, MAP_SIZE + 1 + FOOTER_SIZE * row, ALLEGRO_ALIGN_CENTER, buffer);
		
		delete[] buffer;
	} 

}

static int query_players_cb(void *data, int argc, char **argv, char **azColName)
{
	string key, value;
	player_t player;

	for (int i = 0; i < argc; i++)
	{
		key = string(azColName[i]);
		value = string(argv[i]);

		if (key == "ID")
			player.id = stoi(value);
		else if (key == "LOGIN")
			player.login = value;
		else if (key == "COLOR")
		{
			unsigned char r, g, b;
			r = (unsigned char)stoi(value.substr(0, 2), NULL, 16) & 0xff;
			g = (unsigned char)stoi(value.substr(2, 2), NULL, 16) & 0xff;
			b = (unsigned char)stoi(value.substr(4, 2), NULL, 16) & 0xff;
			player.color = al_map_rgb(r, g, b);
		}
		else
			std::cout << "WTF?" << endl;
	}

	std::cout << endl << "ID=" << player.id << endl;
	std::cout << "LOGIN=" << player.login << endl;
	std::cout << "COLOR=(" << player.color.r * 255 << "," << player.color.g * 255 << "," << player.color.b * 255 << ")" << endl;
	player.login_hash = crc32(0, player.login.c_str(), player.login.length());

	bool test = false;

	do
	{
		player.x = (float)rand() / ((float)RAND_MAX / MAP_SIZE);
		player.y = (float)rand() / ((float)RAND_MAX / (MAP_SIZE - BLOCK_SIZE));

		if (game->players.size())
		{
			for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end() - 1; i++)
				test = test || player.collision(*i);
		}	

	} while (test);

	std::cout << "X=" << player.x << endl;
	std::cout << "Y=" << player.y << endl;

	game->players.push_back(player);

	return 0;
}

int setup_players()
{	
	char * err_msg = 0;

	std::cout << "Connecting to database TheGame.db ...";

	srand(time(NULL));
	
	game->players.clear();
	game->bullets.clear();

	if (sqlite3_open("TheGame.db", &game->db))
	{
		std::cout << "failure: " << sqlite3_errmsg(game->db) << endl;
		return -1;
	}
	else
		std::cout << "success" << endl;;	

	sqlite3 * db = game->db;

	if (sqlite3_exec(db, "SELECT * from USERS", query_players_cb,NULL, &err_msg) != SQLITE_OK)
	{
		std::cout << "SQL error: " << err_msg << endl;
		sqlite3_free(err_msg);
	}
	else
		std::cout << "Finished reading players" << endl;
	
}
void send_state(char * buf)
{
	const unsigned char n = (unsigned char)game->players.size();
	struct server_msg * msg = new server_msg[1];

	ZeroMemory(msg, sizeof server_msg);
	ZeroMemory(buf, 4096);
	
	char * p = buf;
	size_t len = sizeof(server_msg)*n + 1;

	if (len > 4096)
	{
		fprintf(stderr, "Buffer too large!");
		exit(EXIT_FAILURE);
	}

	*p++ = n;

	for (vector<player_t>::iterator i = game->players.begin() ; i < game->players.end(); i++)
	{
		msg->x = (short)i->x;
		msg->y = (short)i->y;
		msg->alpha = (float)i->gun_alpha;
		msg->hp = (short)i->hp;
		msg->flags = 0;
		msg->flags |= (short)i->alive;
		msg->flags |= ((short)i->shot << 1);
		msg->flags |= ((short)(i->ammo == MAX_AMMO) << 2);
		memcpy(p, msg, sizeof server_msg);
		p += sizeof(server_msg);
	}
	sendall(buf, len);
	delete[] msg;
}
int game_loop(void) {

	char * buf = new char[4096];
	while (!al_key_down(&game->klawiatura, ALLEGRO_KEY_ESCAPE) && !game->interrupted)
	{
		al_wait_for_event(game->queue, &game->event);
		if (game->event.type == ALLEGRO_EVENT_TIMER)
		{

			al_get_keyboard_state(&game->klawiatura);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			
			for (auto i = game->players.begin(); i < game->players.end(); i++) // First players move
				i->move();

			
			for (auto j = game->bullets.begin(); j != game->bullets.end(); j++) // Then bullets
			{
				j->move();
				if (!j->alive)
					game->bullets.erase(j);
			}


			for (auto i = game->players.begin(); i < game->players.end(); i++) // Then players shoot 
				i->shoot(&game->bullets, game->shoot);


			check_collisions(); // check collision


			check_hits(); // and check hits


			for (auto i = game->players.begin(); i != game->players.end(); i++) // draw everything
				i->draw();
			for (auto i = game->bullets.begin(); i != game->bullets.end(); i++)
				i->draw();

			send_state(buf); // send new state to players
		}
		if (game->event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			game->interrupted = 1;
			return -1;
		}
		draw_footer(); // draw footer
		al_flip_display(); // update window
	}
	game->interrupted = 1;
	return -1;
}

void clean_exit(int exit_code)
{
	game->bullets.clear();
	game->players.clear();

	al_destroy_timer(game->timer);
	al_destroy_display(game->okno);
	al_destroy_sample(game->theme);
	al_destroy_sample(game->shoot);

	if (game->db)
		sqlite3_close(game->db);
}

void signal_handler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ")" << endl;
	game->interrupted = signum;
}

int main(int argc, char **argv)
{
	std::cout.rdbuf()->pubsetbuf(0, 0);
	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGTERM, signal_handler);

	al_init();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_acodec_addon();

	setup_players();

	al_install_audio();
	al_install_keyboard();
	al_reserve_samples(2);

	if (!(game->theme = al_load_sample("hydrogen.ogg")) || !(game->shoot = al_load_sample("shoot.ogg")))
	{
		Message("Cannot load sample!");
	}

	game->font = al_load_font("Times.ttf", 72, 0);
	game->menu_font = al_load_font("Times.ttf", 36, 0);
	game->foot_font = al_load_font("Times.ttf", FOOTER_SIZE - 4, 0);

	if (!game->font || !game->foot_font)
	{
		Message("Cannot load font!");
		return 1;
	}

	game->footer_rows = game->players.size() & 0x3 ? game->players.size() / 4 + 1: game->players.size() / 4;
	std::cout << "NUMBER OF ROWS = " << game->footer_rows << endl;

	al_set_new_display_flags(ALLEGRO_WINDOWED);
	game->okno = al_create_display(MAP_SIZE, MAP_SIZE + FOOTER_SIZE * game->footer_rows);
	al_set_window_title(game->okno, "The Game");
	game->queue = al_create_event_queue();
	game->timer = al_create_timer(1.0 / FPS);

	//al_hide_mouse_cursor(okno);
	al_start_timer(game->timer);
	al_register_event_source(game->queue, al_get_display_event_source(game->okno));
	al_register_event_source(game->queue, al_get_timer_event_source(game->timer));
	//al_play_sample(theme, 0.7, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
	game->port = argc > 1 ? (unsigned short)atoi(argv[1]) : 8080;

	HTHREAD thread = StartThread(run_server);
	game_loop();

	WaitForThread(thread);
	clean_exit(0);
	return 0;
}