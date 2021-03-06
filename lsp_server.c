#include "lsp_server.h"
#include "lspmessage.pb-c.h"

double epoch_lth = _EPOCH_LTH;
int epoch_cnt = _EPOCH_CNT;
double drop_rate = .05;
//double drop_rate = _DROP_RATE;

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
	char *port = (char*)malloc(5*sizeof(char));
	sprintf(port, "%d", portNum);
	
	lsp_server *server = (lsp_server*)malloc(sizeof(lsp_server));
	server->info = sock_n_bind(port);	

	server->clients.push_back(Connection());	//fill the 0 index so connection id will be a little more intuitive

	pthread_t epoch_counter;				//initialize the epoch counter and run it
	int thread_id = pthread_create(&epoch_counter, NULL, epoch_trigger, (void*)server);

	free(port);
	return server;
}
	
	
	
void* epoch_trigger(void* server){
	
	lsp_server* serverCopy = (lsp_server*)server;
	int i;
	void* buffer = NULL;
	unsigned size;
	
	LSPMessage template_message = LSPMESSAGE__INIT;
	template_message.connid = 1;
	template_message.seqnum = 0;
	template_message.payload.data = NULL;
	template_message.payload.len = 0;
	
	size = lspmessage__get_packed_size(&template_message);
	buffer = malloc(size);
	lspmessage__pack(&template_message, (uint8_t*)buffer);
	
	while(true){
		sleep(epoch_lth);
		for(i = 1; i < serverCopy->clients.size(); i++){
			//if the timeout count > epoch_cnt, disconnect them
			if(serverCopy->clients[i].timeout_cnt++ == epoch_cnt)
				lsp_server_close(serverCopy, i);
				//printf("Client %d timed out\n", i);
				
			if(serverCopy->clients[i].conn_id > 0){
				template_message.connid = i;
				template_message.seqnum = serverCopy->clients[i].message_seq_num;
				memset(buffer, 0, size);
				lspmessage__pack(&template_message, (uint8_t*)buffer);
				
				if((rand() % 100) > 100 * drop_rate){	//send the packet, else drop it
					serv_send(serverCopy->info, buffer, size, serverCopy->clients[i].clientAddr);
					//printf("ICMP packet sent\n");
				}
			}
		}
	}
}

int lsp_server_read(lsp_server* a_srv, void* pld, uint32_t* conn_id)
{
	memset(pld, 0, MAX_PACKET_SIZE);
	LSPMessage *message;
	uint8_t buf[MAX_PACKET_SIZE];
	struct sockaddr src;
	
	int length = serv_recv(a_srv->info, buf, &src, epoch_lth);
	if(length <= 0) return -1;
	
	message = lspmessage__unpack(NULL, length, buf);
	if(message == NULL){
		//printf("Error unpacking message\n");
		lspmessage__free_unpacked(message, NULL);
		return -1;
	}

	bool newMsg = false;
	int i;
	for(i = 0; i < a_srv->clients.size(); i++){		//is this message from someone we know?
		
		if(a_srv->clients[i].conn_id == message->connid){
			*conn_id = message->connid;		//inform the server who has sent the message
			a_srv->clients[i].timeout_cnt = 0;		//reset timeout counter	
			
			if(message->payload.data == NULL){	//ICMP/ack packet; no further action
				return 0;
			}
			if(message->seqnum == a_srv->clients[i].message_seq_num){
				a_srv->clients[i].message_seq_num++;	//update message sequence
				newMsg = true;
			}else 
				return 0;
			
			/* create ack */
			int ack_size;
			uint8_t *buffer;
			LSPMessage msg = LSPMESSAGE__INIT;
			msg.connid = i;
			msg.seqnum = a_srv->clients[i].message_seq_num;
			msg.payload.data = NULL;
			msg.payload.len = 0;
			ack_size = lspmessage__get_packed_size(&msg);
			buffer = (uint8_t*)malloc(ack_size);
			lspmessage__pack(&msg, (uint8_t*)buffer);
			
			//send the ack
			bool sent;
			if((rand() % 100) > 100 * drop_rate)	//send the packet, else drop it
				sent = serv_send(a_srv->info, buffer, ack_size, a_srv->clients[i].clientAddr);
			else sent = true;
			
			/* if this message is new, return it, otherwise the message was just a resent or ack packet */
			if(newMsg){
				length = message->payload.len;	
				for(i = 0; i < length; i++){				//load the message into the buffer we were passed to read
					((char*)pld)[i] = message->payload.data[i];
				}
			}
			free(buffer);
			lspmessage__free_unpacked(message, NULL);
			return length;
		}
	}

	int temp_conn_id;
	if(message->payload.data == NULL)	//this is a new connection request
	{
		for(i = 1; i < a_srv->clients.size(); i++){	//check for an available client slot
			if(a_srv->clients[i].clientAddr.sa_data == src.sa_data){	//client didn't receive the initial connection id; resend		
				break;
			}else if(a_srv->clients[i].conn_id == -1){	//there's an opening in the existing client list; initialize them
				a_srv->clients[i].clientAddr = src;
				a_srv->clients[i].conn_id = i;
				a_srv->clients[i].message_seq_num = 0;
				a_srv->clients[i].timeout_cnt = 0;
				break;
			}
		}
		
		if(i == a_srv->clients.size())	a_srv->clients.push_back(Connection(src, i, 0, 0));	//vector full, add a new one

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
		buffer = (uint8_t*)malloc(response_size);
		lspmessage__pack(&msg, buffer);

		uint8_t response[MAX_PACKET_SIZE];
		memset(response, 0, MAX_PACKET_SIZE);

		if((rand() % 100) > 100 * drop_rate)	//send the packet, else drop it
			serv_send(a_srv->info, buffer, response_size, a_srv->clients[temp_conn_id].clientAddr);
		
		free(buffer);
		//printf("Connection request received, assigned connection id %d\n", temp_conn_id);

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
		//printf("Invalid connection id\n");
		return false;
	}
	
	void* buffer;
	unsigned size;	
	
	LSPMessage msg = LSPMESSAGE__INIT;
	msg.connid = a_srv->clients[connection_id].conn_id;
	msg.seqnum = a_srv->clients[connection_id].message_seq_num++;
	msg.payload.data = (uint8_t*)malloc(sizeof(uint8_t) * length);
	msg.payload.len = length;
	memcpy(msg.payload.data, pld, length * sizeof(uint8_t));
	size = lspmessage__get_packed_size(&msg);
	buffer = malloc(size);
	lspmessage__pack(&msg, (uint8_t*)buffer);

	/* send the packet until ack received */
	int i = 0;
	int ack_size = 0;
	
	uint8_t ack[length];
	memset(ack, 0, length);
	bool sent;
	//sockaddr source;
	
	while(i < epoch_cnt && ack_size <= 0){	//keep trying to send packet up to epoch times
		if((rand() % 100) > 100 * drop_rate)	//send the packet, else drop it
			serv_send(a_srv->info, buffer, size, a_srv->clients[connection_id].clientAddr);
			
	//	sleep(epoch_lth);
		ack_size = serv_recv(a_srv->info, ack, NULL, epoch_lth);
		if(ack_size > 0){
			LSPMessage *message;
			message = lspmessage__unpack(NULL, ack_size, ack);
			/* make sure the ack is from the target client, not some other message */
			if(a_srv->clients[connection_id].conn_id == message->connid){
				sent = true;
				break;
			}
		}
		i++;
	}
	
	free(buffer);
	free(msg.payload.data);
	if(sent){
		////printf("message of length %d bytes sent\n", size);
		return true;
	}else{
		//printf("Host may not be available\n");
		return false;
	}
}



bool lsp_server_close(lsp_server* a_srv, uint32_t connection_id)
{
	if(a_srv->clients[connection_id].conn_id != connection_id){
	//	//printf("Invalid connection id\n");
		return false;
	}
	
//	memset(&a_srv->clients[connection_id].clientAddr, 0, sizeof(struct sockaddr));
	a_srv->clients[connection_id].conn_id = -1;
	a_srv->clients[connection_id].message_seq_num = -1;
	a_srv->clients[connection_id].timeout_cnt = a_srv->clients.size()+1;
	//a_srv->clients[connection_id].payload.data = NULL;

	
	return true;
}
