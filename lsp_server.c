#include "lsp_server.h"
#include "lspmessage.pb-c.h"

double epoch_lth = _EPOCH_LTH;
int epoch_cnt = _EPOCH_CNT;
double drop_rate = _DROP_RATE;

/*
 *
 *
 *				LSP RELATED FUNCTIONS
 *
 *
 */  

void lsp_set_epoch_lth(double lth){epoch_lth = lth;}
void lsp_set_epoch_cnt(int cnt){epoch_cnt = cnt;}
void lsp_set_drop_rate(double rate){drop_rate = rate;}

/*
 *
 *
 *				SERVER RELATED FUNCTIONS
 *
 *
 */  

lsp_server* lsp_server_create(const char* port)
{
	lsp_server *server = malloc(sizeof(lsp_server));
	server->info = sock_n_bind(port);	
	
	return server;
}
	

	
int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{
	memset(pld, 0, 1024);
	LSPMessage *message;
	uint8_t buf[1024];
	struct sockaddr src;
	
	int length = serv_recv(a_srv->info, buf, &src);
	
	if(length <= 0) return -1;
	
//printf("%d\n", a_srv->clients[0].message_seq_num);
	
	message = lspmessage__unpack(NULL, length, buf);
	if(message == NULL){
		printf("Error unpacking message\n");
		return -1;
	}
	
	int i;
	length = strlen(message->payload.data);
	if(length == 0){	//new connection request
		/*for(i = 0; i < 10; i++){
			if(a_srv->clients[i].clientAddr.sa_data == NULL){
				a_srv->clients[i].clientAddr = src;
				a_srv->clients[i].conn_id = i;
				a_srv->clients[i].message_seq_num = 0;
				break;
			}
		}*/
	}	
	
	i = 0;	//temporary 4 lines of code so you guys can use this
	a_srv->clients[i].clientAddr = src;
	a_srv->clients[i].conn_id = i;
	a_srv->clients[i].message_seq_num = 0;
	
	char* temp = pld;
	for(i = 0; i < length; i++){
		*(char*)temp++ = message->payload.data[i];
	}
	
	lspmessage__free_unpacked(message, NULL);
	return length;
}



bool lsp_server_write(lsp_server* a_srv, void* pld, int length, uint32_t conn_id)
{
	void* buffer;
	unsigned size;	
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = a_srv->clients[0].conn_id;
	msg.seqnum = ++a_srv->clients[0].message_seq_num;
	msg.payload.data = malloc(sizeof(uint8_t) * length);
	msg.payload.len = length;
	memcpy(msg.payload.data, pld, length * sizeof(uint8_t));
	
	size = lspmessage__get_packed_size(&msg);
	buffer = malloc(size);
	lspmessage__pack(&msg, buffer);

	bool sent = serv_send(a_srv->info, buffer, size, a_srv->clients[0].clientAddr);
	
	free(buffer);
	free(msg.payload.data);
	if(sent){
		printf("message of length %d bytes sent\n", size);
		return true;
	}else{
		printf("Host may not be available\n");
		return false;
	}
}



bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id)
{
	if(serv_close(a_srv->info))
		printf("Connection closed\n");
	free(a_srv);
	return true;
}
