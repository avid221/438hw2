#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/poll.h>
#include "lspmessage.pb-c.h"


typedef struct{

	struct addrinfo *server;
	int serverFD;

}UDPserver;

UDPserver *sock_n_bind(const char* port);

int serv_listen(UDPserver *a_srv, int max_clients);

bool serv_send(UDPserver *a_srv, void* message, int length, struct sockaddr dest);

int serv_recv(UDPserver *a_srv, void* payload, struct sockaddr* src);

bool serv_close(UDPserver *a_srv);

typedef struct{

struct addrinfo *server;
int serverFD;

}UDPclient;

UDPclient *sock_n_conn(const char* src, const char* port);

bool cli_send(UDPclient *a_client, LSPMessage *payload, int length);

int cli_recv(UDPclient *a_client, uint8_t *payload);

bool cli_close(UDPclient *a_client);
