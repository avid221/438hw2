#pragma once

#include "lsp.h"

typedef struct
{
	UDPclient* info;
	int message_seq_num;
	int conn_id;	//i elected not to use the connection struct here because conn_id should not change for the client
} lsp_client;

lsp_client* lsp_client_create(const char* dest, int port);	//TODO: make port an int
int lsp_client_read(lsp_client* a_client, uint8_t* payload);
bool lsp_client_write(lsp_client* a_client, uint8_t* payload, int length);
bool lsp_client_close(lsp_client* a_client);
