#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#define EXIT_FAILURE 1
#endif

#include "bullet.h"
#include "server.h"

using namespace std;

TG_SOCKET server_socket = INVALID_SOCKET;

void setup_server(unsigned short port)
{
	int res = 0;
	TG_SOCKET tmp_socket;

	struct sockaddr_in server_info;
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(port);
	server_info.sin_addr.s_addr = INADDR_ANY;
	
	tmp_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if (tmp_socket == INVALID_SOCKET) {
#ifdef WIN32
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
#else
		printf("Error at socket(): %ld\n", 1);
#endif
		exit(EXIT_FAILURE);
	}

	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	if (setsockopt(tmp_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
		perror("Error");
	}

	if (bind(tmp_socket, (struct sockaddr *)&server_info, sizeof(server_info)) == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(tmp_socket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	server_socket = tmp_socket;
}

vector<player_t>::iterator find(size_t hash)
{
	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		if (i->login_hash == hash)
			return i;
	}
	return game->players.end();
}

vector<player_t>::iterator find_by_ip(struct sockaddr_in & p)
{
	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		if (!memcmp(i->dane, &p, sizeof sockaddr_in))
			return i;
	}
	return game->players.end();
}

void socket_init()
{
#ifdef WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		exit(-1);
	}
#else
#endif
}

void sendall(char * buf, size_t len)
{
	for (vector<player_t>::iterator i = game->players.begin(); i < game->players.end(); i++)
	{
		if (i->connected)
		{
			if (sendto(server_socket, buf, len, 0, (struct sockaddr*)i->dane, sizeof sockaddr_in) == SOCKET_ERROR)
			{
				//printf("sendto() failed with error code : %d\n", WSAGetLastError());
				printf("[-] %s disconnected\n", i->login);
				i->connected = false;
			}
		}
	}
}

struct client_msg
{
	size_t login_hash;
	char choices;
};

void run_server(void) {
	int res = 0;

	struct client_msg * msg;
	struct sockaddr_in client;
	char buf[10];
	size_t len = 0;

	int cl_len = sizeof client;

	ZeroMemory(&client, sizeof client);

	setup_server(game->port);

	std::cout << "Server started with SOCKET " << server_socket << endl << endl;
	
	while (!game->interrupted)
	{
			ZeroMemory(buf, 10);
			if ((len = recvfrom(server_socket, buf, 5, 0, (struct sockaddr*)&client, &cl_len)) == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAETIMEDOUT)
					continue;

				vector<player_t>::iterator pl = find_by_ip(client);

				if (pl != game->players.end())
				{
					if (pl->connected == true)
						printf("[-] %s disconnected\n", pl->login.c_str());
					pl->connected = false;
				}


				continue;
			}

			msg = (struct client_msg*)(buf);

			vector<player_t>::iterator pl = find(msg->login_hash);

			if (pl != game->players.end())
			{
				memcpy(pl->dane, &client, sizeof(struct sockaddr_in));
				pl->state->choices = (unsigned char)msg->choices;
				if (pl->connected == false)
					printf("[+] %s connected!\n", pl->login.c_str());
				pl->connected = true;
			}
		//}
	}
	closesocket(server_socket);
	WSACleanup();
	std::cout << "Server closed" << endl;
	return;
}
