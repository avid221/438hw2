#pragma once

#include "lsp.h"
#include <vector>

struct Connection
{
	struct sockaddr clientAddr;
	int conn_id;
	int message_seq_num;
	unsigned timeout_cnt;
	
	Connection(){
		conn_id = -1;
		message_seq_num = -1;
		timeout_cnt = 11;
	};
	
	Connection(struct sockaddr _clientAddr, int _conn_id, int _message_seq_num, unsigned _timeout_cnt){
		clientAddr = _clientAddr;
		conn_id = _conn_id;
		message_seq_num = _message_seq_num;
		timeout_cnt = _timeout_cnt;
	};
};


typedef struct
{
	UDPserver* info;
	std::vector<Connection> clients;
	//Connection clients[MAX_CLIENTS];
} lsp_server;


lsp_server* lsp_server_create( int port);
int  lsp_server_read(lsp_server* a_srv, void* payload, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, void* payload, int length, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);

/* private functions */
void* epoch_trigger(void* server);
