#include "stdafx.h"
#include "bullet.h"
#include "server.h"
#include <Windows.h>

#define FOR(x) for (size_t i = 0; i < (x).size(); i++)

using namespace std;

ALLEGRO_KEYBOARD_STATE klawiatura;
ALLEGRO_DISPLAY * okno;
ALLEGRO_FONT * font, *foot_font, *menu_font;
ALLEGRO_TIMER * timer;
ALLEGRO_EVENT_QUEUE * queue;
ALLEGRO_EVENT event;
ALLEGRO_SAMPLE *theme, *shoot;
size_t * poly = new size_t[256];

vector < player > players;
list < bullet > bullets;
char ** logins;

void Message(const char * a)
{
	MessageBox(NULL, a, "Error", MB_OK);
}
void check_collisions()
{
	for (size_t i = 0; i < players.size(); i++)
	{
		for (size_t j = i + 1; j < players.size(); j++)
		{
			if (players[i].collision(players[j]))
			{
				float v_x_1 = players[i].v_x;
				float v_x_2 = players[j].v_x;
				float v_y_1 = players[i].v_y;
				float v_y_2 = players[j].v_y;
				
				players[i].v_x = v_x_2;
				players[i].v_y = v_y_2;
				players[j].v_x = v_x_1;
				players[j].v_y = v_y_1;

				players[i].step();
				players[j].step();
			}
		}
	}
}
void check_hits()
{
	for (auto i = players.begin(); i != players.end(); i++)
	{
		for (auto j = bullets.begin(); j != bullets.end(); j++)
		{

			if (i->is_in(j->x, j->y) && j->alive && i->alive)
			{
				vi owner = find(j->owner_id);
				if (owner == players.end())
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
					
				bullets.erase(j);
			}
		}
	}
}
void draw_footer()
{
	for (size_t i = 0; i < players.size(); i++)
	{

		if (players[i].alive)
		{
			al_draw_filled_rectangle(MAP_SIZE / players.size() * i, MAP_SIZE + 2, MAP_SIZE / players.size() * i + MAP_SIZE / players.size() * players[i].hp / HP, MAP_SIZE + FOOTER_SIZE, players[i].color);
		}
		char * buffer = new char[100];
		sprintf_s(buffer, 100, "[%c] %s [%d]", (players[i].connected) ? '+' : '-', logins[i], players[i].points);
		al_draw_line(MAP_SIZE / players.size() * i, MAP_SIZE + 1, MAP_SIZE / players.size() * i + MAP_SIZE / players.size() * players[i].ammo / MAX_AMMO, MAP_SIZE + 1, al_map_rgb(255, 255, 255), 2);
		al_draw_text(foot_font, COLOR_WHITE, MAP_SIZE / players.size() * i + MAP_SIZE / players.size() / 2, MAP_SIZE + 1, ALLEGRO_ALIGN_CENTER, buffer);

		delete[] buffer;
	}
}
void setup_players()
{	
	printf("Reading config file...\n");
	srand(time(NULL));
	players.clear();
	bullets.clear();
	FILE * fd = fopen("poly.bin", "rb");
	fread((void*)poly, 1, 1024, fd);
	fclose(fd);

	if (!(fd = fopen("players.cfg", "r")))
		exit(EXIT_FAILURE);
	size_t n = 0;
	while (!feof(fd))
	{
		char ch = fgetc(fd);
		if (ch == '\n')
			n++;
	}
	rewind(fd);
	logins = new char*[n];

	for (size_t i = 0; i < n; i++)
	{
		logins[i] = new char[11];
		ZeroMemory(*(logins + i), 11);
	}
		
	
	char buffer[100];
	char * login = new char[12];
	ZeroMemory(login, 12);
	ZeroMemory(buffer, 100);
	size_t it = 0;
	while (fgets(buffer,100, fd) != NULL)
	{
		char * p = strchr(buffer, '#');

		if (p == NULL)
			exit(EXIT_FAILURE);

		strncpy(logins[it], buffer, ((p-buffer) <= 10) ? (p-buffer) : 10);
		int r, g, b;
		sscanf(p + 1, "%2x%2x%2x", &r, &g, &b);
		size_t login_hash = crc32(0, logins[it], 10);

		printf("[%d] %s PIN: %08x {R: %d G: %d B: %d}\n",it, logins[it++], login_hash, r, g, b);

		players.push_back(player());
		players[players.size() - 1].color = al_map_rgb(r & 0xff, g & 0xff, b & 0xff);
		players[players.size() - 1].login_hash = login_hash;
		bool test;
		do
		{
			test = false;
			players[players.size() - 1].x = (float)rand() / ((float)RAND_MAX / MAP_SIZE);
			players[players.size() - 1].y = (float)rand() / ((float)RAND_MAX / (MAP_SIZE-BLOCK_SIZE));

			for (size_t i = 0; i < players.size() - 1; i++)
				test = test || players[players.size()-1].collision(players[i]);

		} while (test);
		ZeroMemory(buffer, 100);
	}
	printf("Done!\n");
}
void send_state(char * buf)
{
	const unsigned char n = (unsigned char)players.size();
	struct server_msg * msg = new server_msg[1];
	ZeroMemory(msg, sizeof server_msg);
	ZeroMemory(buf, sizeof 4096);
	char * p = buf;
	size_t len = sizeof(server_msg)*n + 1;
	if (len > 4096)
	{
		fprintf(stderr, "Buffer too large!");
		exit(EXIT_FAILURE);
	}
	*p++ = n;
	for (unsigned char i = 0; i < n; i++)
	{
		msg->x = (short)players[i].x;
		msg->y = (short)players[i].y;
		msg->alpha = (float)players[i].gun_alpha;
		msg->hp = (short)players[i].hp;
		msg->flags = 0;
		msg->flags |= (short)players[i].alive;
		msg->flags |= ((short)players[i].shot << 1);
		msg->flags |= ((short)(players[i].ammo == MAX_AMMO) << 2);
		memcpy(p, msg, sizeof server_msg);
		p += sizeof(server_msg);
	}
	sendall(buf, len);
	delete[] msg;
}
int game(void) {

	setup_players();
	size_t counter = 0;
	char * buf = new char[4096];
	while (!al_key_down(&klawiatura, ALLEGRO_KEY_ESCAPE))
	{
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			al_get_keyboard_state(&klawiatura);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			
			for (auto i = players.begin(); i < players.end(); i++)
				i->move();
			
			for (auto j = bullets.begin(); j != bullets.end(); j++)
			{
				j->move();
				if (!j->alive)
					bullets.erase(j);
			}

			for (auto i = players.begin(); i < players.end(); i++)
				i->shoot(bullets, shoot);

			check_collisions();
			check_hits();

			for (auto i = players.begin(); i != players.end(); i++)
				i->draw();
			for (auto i = bullets.begin(); i != bullets.end(); i++)
				i->draw();

			send_state(buf);
		}
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			return -1;
		draw_footer();
		al_flip_display();
	}
	return -1;
}
int main(void)
{
	al_init();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_acodec_addon();

	al_install_audio();
	al_install_keyboard();
	al_reserve_samples(2);
	if (!(theme = al_load_sample("hydrogen.ogg")) || !(shoot = al_load_sample("shoot.ogg")))
	{
		Message("Cannot load sample!");
	}

	font = al_load_font("Times.ttf", 72, 0);
	menu_font = al_load_font("Times.ttf", 36, 0);
	foot_font = al_load_font("Times.ttf", FOOTER_SIZE - 4, 0);

	if (!font || !foot_font)
	{
		MessageBox(NULL, "Cannot load font!", "Error", MB_OK);
		return 1;
	}

	al_set_new_display_flags(ALLEGRO_WINDOWED);
	okno = al_create_display(MAP_SIZE, MAP_SIZE + FOOTER_SIZE);
	al_set_window_title(okno, "The Game");
	queue = al_create_event_queue();
	timer = al_create_timer(1.0 / FPS);

	//al_hide_mouse_cursor(okno);
	al_start_timer(timer);
	al_register_event_source(queue, al_get_display_event_source(okno));
	al_register_event_source(queue, al_get_timer_event_source(timer));
	//al_play_sample(theme, 0.7, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_server, NULL, 0, NULL);
	game();

	bullets.clear();
	players.clear();
	al_destroy_timer(timer);
	al_destroy_display(okno);
	al_destroy_sample(theme);
	al_destroy_sample(shoot);
	return 0;
}