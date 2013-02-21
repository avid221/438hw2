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

lsp_server* lsp_server_create(int portNum)
{
	char *port = malloc(4*sizeof(char));
	sprintf(port, "%d", portNum);
	
	lsp_server *server = malloc(sizeof(lsp_server));
	server->info = sock_n_bind(port);	
	
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		server->clients[i].conn_id = -1;
		server->clients[i].message_seq_num = -1;
	}
	
	free(port);
	return server;
}
	

	
int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{
	memset(pld, 0, MAX_PACKET_SIZE);
	LSPMessage *message;
	uint8_t buf[MAX_PACKET_SIZE];
	struct sockaddr src;
	
	int length = serv_recv(a_srv->info, buf, &src);
	if(length <= 0) return -1;
	
	message = lspmessage__unpack(NULL, length, buf);
	if(message == NULL){
		printf("Error unpacking message\n");
		lspmessage__free_unpacked(message, NULL);
		return -1;
	}
	
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){	//is this message from someone we know?
		if(a_srv->clients[i].conn_id == message->connid){
			*conn_id = message->connid;	//inform the server who has sent the message
			length = strlen(message->payload.data);		
			
			for(i = 0; i < length; i++){	//load the message into the buffer we were passed to read
				*(char*)pld++ = message->payload.data[i];
			}
			lspmessage__free_unpacked(message, NULL);
			return length;
		}
	}

	int temp_conn_id;
	if(message->payload.data == NULL){	//this is a new connection request
		for(i = 1; i < MAX_CLIENTS; i++){	//check for an available client slot
			if(a_srv->clients[i].clientAddr.sa_data == src.sa_data){
				printf("Duplicate connection request received\n");
				lspmessage__free_unpacked(message, NULL);
				return -2;	//connection request from an existing client.  NOTE: if 2 clients join and client 1 drops right when client 2 resends a connection request, client 2 will be double admitted. Rare occurence though
			}
			if(strlen(a_srv->clients[i].clientAddr.sa_data) == 0){	//there's an opening in the existing client list
				a_srv->clients[i].clientAddr = src;
				a_srv->clients[i].conn_id = i;
				a_srv->clients[i].message_seq_num = 0;
				break;
			}
		}
		if(i == 11){
			printf("Client limit reached\n");
			//need to resize client array or add to a queue
			lspmessage__free_unpacked(message, NULL);
			return -1;
		}
		//send the client their conn_id
		temp_conn_id = i;
		LSPMessage msg = LSPMESSAGE__INIT;
		uint8_t* buffer;
		unsigned response_size;
		
		msg.connid = temp_conn_id;
		msg.seqnum = 0;
		msg.payload.data = NULL;
		msg.payload.len = 0;
		response_size = lspmessage__get_packed_size(&msg);
		buffer = malloc(response_size);
		lspmessage__pack(&msg, buffer);

		uint8_t response[256];
		memset(response, 0, 256);
		unsigned ack_size = 0;
		i = 0;
		while(i < 5 && ack_size <= 0){	//keep trying to send connection request up to 5 times
			bool sent = serv_send(a_srv->info, buffer, response_size, a_srv->clients[temp_conn_id].clientAddr);
			sleep(1);	
			ack_size = serv_recv(a_srv->info, response, &src);	//check for an ack
			if(ack_size > 0){	//make sure the ack is actually an ack, not another connection request
				LSPMessage *message;
				message = lspmessage__unpack(NULL, ack_size, response);
				if(message->connid == temp_conn_id) break;		//the client got its conn_id, and responded appropriately
				else continue;	//some other client got in the way, but we don't want it to count as a failed response
			}
			i++;
		}
		free(buffer);
		printf("Connection request received, assigned connection id %d\n", temp_conn_id);
		*conn_id = temp_conn_id;	//inform the server what conn_id it has just assigned
		lspmessage__free_unpacked(message, NULL);
		return 0;
	}	
	//stray packet that nobody loves
	*conn_id = -1;
	return -1;
}



bool lsp_server_write(lsp_server* a_srv, void* pld, int length, uint32_t connection_id)
{	
	if(a_srv->clients[connection_id].conn_id != connection_id){
		printf("Invalid connection id\n");
		return false;
	}
	
	void* buffer;
	unsigned size;	
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = a_srv->clients[connection_id].conn_id;
	msg.seqnum = ++a_srv->clients[connection_id].message_seq_num;
	msg.payload.data = malloc(sizeof(uint8_t) * length);
	msg.payload.len = length;
	memcpy(msg.payload.data, pld, length * sizeof(uint8_t));
	
	size = lspmessage__get_packed_size(&msg);
	buffer = malloc(size);
	lspmessage__pack(&msg, buffer);

	bool sent = serv_send(a_srv->info, buffer, size, a_srv->clients[connection_id].clientAddr);
	
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



bool lsp_server_close(lsp_server* a_srv, uint32_t connection_id)
{
	if(a_srv->clients[connection_id].conn_id != connection_id){
		printf("Invalid connection id\n");
		return false;
	}
	
	memset(&a_srv->clients[connection_id].clientAddr, 0, sizeof(struct sockaddr));
	a_srv->clients[connection_id].conn_id = -1;
	a_srv->clients[connection_id].message_seq_num = -1;

	return true;
}




