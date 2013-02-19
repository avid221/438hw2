#pragma once

#include "lsp.h"

typedef struct
{
	struct sockaddr clientAddr;
	int conn_id;
	int message_seq_num;
} Connection;

typedef struct
{
	UDPserver* info;
	Connection clients[10];
} lsp_server;


lsp_server* lsp_server_create(const char* port);	//TODO: make port an int
int  lsp_server_read(lsp_server* a_srv, void* payload, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* payload, int length, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);
