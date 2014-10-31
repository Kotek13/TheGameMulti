#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "bullet.h"
#include "server.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8081
SOCKET server_socket = INVALID_SOCKET;

size_t crc32(size_t hash, char * string, size_t len)
{
	for (char * i = string; len; len--, i++)
		hash = (hash >> 8) ^ (poly[((hash & 0xff) ^ *i) & 0xff]);
	return hash;
}

void WSA_init(WSADATA& wsaData)
{
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		exit(-1);
	}
}

void get_ip(sockaddr_in& server_info)
{
	char ac[80];
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
		fprintf(stderr, "Error when getting local host name.\n");
		exit(EXIT_FAILURE);
	}

	struct hostent *phe = gethostbyname(ac);
	if (phe == 0) {
		fprintf(stderr, "Yow! Bad host lookup.\n");
		exit(EXIT_FAILURE);
	}
	int VM = 0x0138A8C0;
	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		if (*(int*)(&addr) != VM)
		{
			fprintf(stderr, "Trying IP: %s...", inet_ntoa(addr));
			memcpy(&server_info.sin_addr, &addr, 4);
		}
		
	}
	//server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
}

void setup_server(SOCKET& server_socket)
{
	int res = 0;
	SOCKET tmp_socket;
	struct sockaddr_in server_info;
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(PORT);
	get_ip(server_info);
	
	tmp_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if (tmp_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	if (bind(tmp_socket, (struct sockaddr *)&server_info, sizeof(server_info)) == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(tmp_socket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	server_socket = tmp_socket;
}

vi find(size_t hash)
{
	for (vi i = players.begin(); i < players.end(); i++)
	{
		if (i->login_hash == hash)
			return i;
	}
	return players.end();
}

vi find_by_ip(struct sockaddr_in & p)
{
	for (vi i = players.begin(); i < players.end(); i++)
	{
		if (!memcmp(i->dane, &p, sizeof sockaddr_in))
			return i;
	}
	return players.end();
}

void sendall(char * buf, size_t len)
{
	for (vi i = players.begin(); i < players.end(); i++)
	{
		if (i->connected)
		{
			if (sendto(server_socket, buf, len, 0, (struct sockaddr*)i->dane, sizeof sockaddr_in) == SOCKET_ERROR)
			{
				//printf("sendto() failed with error code : %d\n", WSAGetLastError());
				printf("[-] %s disconnected\n", logins[distance(players.begin(), i)]);
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

void run_server() {
	WSADATA wsaData;
	WSA_init(wsaData);
	int res = 0;
	struct client_msg * msg;
	struct sockaddr_in client;
	char buf[10];
	size_t len = 0;
	int cl_len = sizeof client;
	ZeroMemory(&client, sizeof client);
	setup_server(server_socket);
	printf("server started.\n\nLet's play!\n\n");
	while (1)
	{
		ZeroMemory(buf, 10);
		if ((len = recvfrom(server_socket, buf, 5, 0, (struct sockaddr*)&client, &cl_len)) == SOCKET_ERROR)
		{
			vi pl = find_by_ip(client);

			if (pl != players.end())
			{
				if (pl->connected == true)
					printf("[-] %s disconnected\n", logins[distance(players.begin(), pl)]);
				pl->connected = false;
			}
				

			continue;
		}
			
		
		msg = (struct client_msg*)(buf);
		
		vi pl = find(msg->login_hash);

		if (pl != players.end())
		{
			memcpy(pl->dane, &client, sizeof(struct sockaddr_in));
			pl->state->choices = (unsigned char)msg->choices;
			if (pl->connected == false)
				printf("[+] %s connected!\n", logins[distance(players.begin(), pl)]);
			pl->connected = true;
		}
	}
	closesocket(server_socket);
	WSACleanup();
	return;
}
