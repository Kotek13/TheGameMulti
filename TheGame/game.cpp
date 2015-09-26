#include "game.h"

void Message(const char * a)
{
	MessageBox(NULL, a, "Error", MB_OK);
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
				if (i->login_hash == j->owner_id)
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
			al_draw_filled_rectangle(game->settings.map_size / 4 * col, game->settings.map_size + 2 + row * game->settings.footer_size, game->settings.map_size / 4 * col + game->settings.map_size / 4 * i->hp / game->settings.hp, game->settings.map_size + game->settings.footer_size * (row + 1), i->color);

		// ammo bar
		al_draw_line(game->settings.map_size / 4 * col, game->settings.map_size + 1 + game->settings.footer_size * row, game->settings.map_size / 4 * col + game->settings.map_size / 4 * i->ammo / game->settings.max_ammo, game->settings.map_size + 1 + game->settings.footer_size * row, al_map_rgb(255, 255, 255), 2);

		// login
		char * buffer = new char[100];
		sprintf_s(buffer, 100, "[%c] %s [%d]", i->connected ? '+' : '-', i->login.c_str(), i->points);

		al_draw_text(game->foot_font, COLOR_WHITE, game->settings.map_size / 4 * col + game->settings.map_size / 4 / 2, game->settings.map_size + 1 + game->settings.footer_size * row, ALLEGRO_ALIGN_CENTER, buffer);

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
		else if (key == "POINTS")
			player.points = stoi(value);

		else
			std::cout << "WTF?" << endl;
	}

	player.login_hash = crc32(0, player.login.c_str(), player.login.length());

	printf("[%d] %-20s (%08x) has %-10d points\n", player.id, player.login.c_str(), player.login_hash, player.points);

	bool test = false;

	do
	{
		player.x = (float)rand() / ((float)RAND_MAX / game->settings.map_size);
		player.y = (float)rand() / ((float)RAND_MAX / (game->settings.map_size - game->settings.block_size));

		if (game->players.size())
		{
			for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end() - 1; i++)
				test = test || player.collision(*i);
		}

	} while (test);

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

	std::cout << "Starting reading players" << endl;
	sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &err_msg);

	if (sqlite3_exec(db, "SELECT * from USERS", query_players_cb, NULL, &err_msg) != SQLITE_OK)
	{
		std::cout << "SQL error: " << err_msg << endl;
		sqlite3_free(err_msg);
	}
	else
		std::cout << "Finished reading players" << endl << endl;

	return 0;
}

int connect_point_server()
{
	socket_init();
	TG_SOCKET tmp_socket = INVALID_SOCKET;
	struct sockaddr_in points_addr;

	memset(&points_addr, 0, sizeof(struct sockaddr_in));
	points_addr.sin_family = AF_INET;
	points_addr.sin_port = htons(12345);
	points_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	tmp_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (tmp_socket == INVALID_SOCKET) {
#ifdef WIN32
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
#else
		printf("Error at socket(): %ld\n", 1);
#endif
		return 1;
	}

	if (connect(tmp_socket, (sockaddr*)&points_addr, sizeof(points_addr)) != 0)
	{
		printf("Cannot connect to points server!\n");
		return 1;
	}
	game->points_server = tmp_socket;
	return 0;
}

void setup_window()
{
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_acodec_addon();
	al_install_audio();
	al_install_keyboard();
	al_reserve_samples(2);

	if (!(game->theme = al_load_sample("hydrogen.ogg")) || !(game->shoot = al_load_sample("shoot.ogg")))
	{
		Message("Cannot load sample!");
	}

	game->font = al_load_font("Times.ttf", 72, 0);
	game->menu_font = al_load_font("Times.ttf", 36, 0);
	game->foot_font = al_load_font("Times.ttf", game->settings.footer_size - 4, 0);

	if (!game->font || !game->foot_font)
	{
		Message("Cannot load font!");
	}

	game->footer_rows = game->players.size() & 0x3 ? game->players.size() / 4 + 1 : game->players.size() / 4;

	al_set_new_display_flags(ALLEGRO_WINDOWED);

	game->window = al_create_display(game->settings.map_size, game->settings.map_size + game->settings.footer_size * game->footer_rows);

	al_set_window_title(game->window, "The Game");
	al_set_window_position(game->window, -100, 3);

	game->queue = al_create_event_queue();
	game->timer = al_create_timer(1.0 / game->settings.fps);

	//al_hide_mouse_cursor(window);
	al_start_timer(game->timer);
	al_register_event_source(game->queue, al_get_display_event_source(game->window));
	al_register_event_source(game->queue, al_get_timer_event_source(game->timer));
	//al_play_sample(theme, 0.7, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
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

	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		msg->x = (short)i->x;
		msg->y = (short)i->y;
		msg->alpha = (float)i->gun_alpha;
		msg->hp = (short)i->hp;
		msg->flags = 0;
		msg->flags |= (short)i->alive;
		msg->flags |= ((short)i->shot << 1);
		msg->flags |= ((short)(i->ammo == game->settings.max_ammo) << 2);
		memcpy(p, msg, sizeof server_msg);
		p += sizeof(server_msg);
	}

	sendall(buf, len);
	delete[] msg;
}

struct point_buffer
{
	unsigned int id;
	unsigned int points;
};

void send_points()
{
	int n = game->players.size();

	char buffer[2000] = { 0 };



	char * p = buffer;

	*p++ = (unsigned char)n;

	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++, p+= sizeof(point_buffer))
	{
		struct point_buffer * t = (struct point_buffer *)p;
		t->id = i->id;
		t->points = i->points;
	}

	send(game->points_server, buffer, p - buffer, 0);

}

void save_points_quiet()
{

	char sql[2000];

	char * zErrMsg = NULL;

	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		ZeroMemory(sql, sizeof(sql));

		sprintf(sql, "UPDATE USERS set POINTS = %d where (ID = %d and POINTS < %d)", i->points, i->id, i->points);

		if (sqlite3_exec(game->db, sql, NULL, NULL, &zErrMsg) != SQLITE_OK)
		{
			printf("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}
}

void game_loop(void) {

	char * buf = new char[4096];
	while (!al_key_down(&game->keyboard, ALLEGRO_KEY_ESCAPE) && !game->interrupted)
	{
		al_wait_for_event(game->queue, &game->event);
		if (game->event.type == ALLEGRO_EVENT_TIMER && !game->interrupted)
		{

			al_get_keyboard_state(&game->keyboard);
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
			if (game->counter++ > 5 * game->settings.fps)
			{
				//send_points();
				save_points_quiet();
				game->counter = 0;
			}
		}
		if (game->event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			game->interrupted = 1;
			return;
		}
		if (!game->interrupted)
		{
			draw_footer(); // draw footer
			al_flip_display(); // update window
		}
	}
	game->interrupted = 1;
}
