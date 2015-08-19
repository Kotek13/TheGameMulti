#ifndef _SERVER_H
#define _SERVER_H

#ifdef WIN32
typedef SOCKET TG_SOCKET;
#else
typedef int TG_SOCKET;
#define INVALID_SOCKET -1;
#endif

vector<player_t>::iterator find(size_t hash);

void sendall(char * buf, size_t len);

struct server_msg
{
	short x;
	short y;
	float alpha;
	short hp;
	short flags;
};

void run_server(void);
void socket_init();

#endif