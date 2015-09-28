#include "stdafx.h"

game_t * game = new game_t;

using namespace std;

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

static int save_points_cb(void *data, int argc, char **argv, char **azColName)
{
	string key, value;
	int id, points;
	string login;

	for (int i = 0; i < argc; i++)
	{
		key = string(azColName[i]);
		value = string(argv[i]);

		if (key == "ID")
			id = stoi(value);
		else if (key == "LOGIN")
			login = value;
		else if (key == "POINTS")
			points = stoi(value);
		else
			std::cout << "WTF?" << endl;
	}

	printf("[%d] %-20s finished with %-10d points\n", id, login.c_str(), points);
	return 0;
}

void save_points()
{

	char sql[2000];

	char * zErrMsg = NULL;
	std::cout << endl << "Saving points for players" << endl;

	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		ZeroMemory(sql, sizeof(sql));

		sprintf(sql, "UPDATE %s set POINTS = %d where (ID = %d and POINTS < %d); SELECT ID,LOGIN,POINTS FROM %s where ID = %d", game->settings.table_name.c_str(), i->points, i->id, i->points, game->settings.table_name.c_str(), i->id);
		
		if (sqlite3_exec(game->db, sql, save_points_cb, NULL, &zErrMsg ) != SQLITE_OK)
		{
			printf("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}

	printf("Finished saving points\n");
}

void clean_exit(int exit_code)
{
	save_points();

	game->bullets.clear();
	game->players.clear();

	if (game->timer)
		al_destroy_timer(game->timer);
	
	if (game->window)
		al_destroy_display(game->window);

	if (game->theme)
		al_destroy_sample(game->theme);

	if (game->theme)
		al_destroy_sample(game->shoot);

	if (game->db)
		sqlite3_close(game->db);
}

void signal_handler(int signum)
{
	std::cout << endl << "Interrupt signal (" << signum << ")" << endl;
	game->interrupted = signum;
}


int main(int argc, char **argv)
{
	std::cout.rdbuf()->pubsetbuf(0, 0);
	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGTERM, signal_handler);

	game->interrupted = 0;

	al_init();

	game->settings.load_xml("settings.xml");
	socket_init();
	setup_players();
	setup_window();
	game->port = game->settings.port;
	//game->port = argc > 1 ? (unsigned short)atoi(argv[1]) : 8080;

	HTHREAD thread = StartThread(run_server);
	game_loop();

	WaitForThread(thread);

	std::cout << "Thread finished" << endl;

	clean_exit(0);
	return 0;
}