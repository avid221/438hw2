#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/poll.h>

#define MAX_CLIENTS 11	//we don't want conn_id = 0, and it would be risky to subtract 1 from all array indexes, so the array is size MAX_CLIENTS, but in reality we only have 10 clients
#define MAX_PACKET_SIZE 1024

typedef struct{

	struct addrinfo *server;
	int serverFD;

}UDPserver;

UDPserver *sock_n_bind( const char* portNum);

int serv_listen(UDPserver *a_srv, int max_clients);

bool serv_send(UDPserver *a_srv, void* message, int length, struct sockaddr dest);

int serv_recv(UDPserver *a_srv, void* payload, struct sockaddr* src, int timeout);

bool serv_close(UDPserver *a_srv);

typedef struct{

struct addrinfo *server;
int serverFD;

}UDPclient;

UDPclient *sock_n_conn(const char* src,  const char* portNum);

bool cli_send(UDPclient *a_client, void *payload, int length);

int cli_recv(UDPclient *a_client, uint8_t *payload, int timeout);

bool cli_close(UDPclient *a_client);


