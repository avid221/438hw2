#include "udp.h"

/*
	makes use of the traditional UDP layer to create the 
	server & client	for our purposes
	
*/


UDPserver* sock_n_bind( const char* port){
	int serverFD, clientFD, buf_size, status, i;
	struct addrinfo settings, *temp_addr_info, *server;
	//char newport[5], *port = &newport;
	//itoa(portNum, port, 10);
	
	UDPserver *this = malloc(sizeof(UDPserver));

    memset(&settings, 0, sizeof(settings));
    settings.ai_family = AF_INET;
    settings.ai_socktype = SOCK_DGRAM;
    settings.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, port, &settings, &server)) != 0){
        perror("Failed to get address info");
        return NULL;
    }
    
	int yes = 1;
	
    for(temp_addr_info = server; temp_addr_info != NULL; temp_addr_info = temp_addr_info->ai_next) {
        serverFD = socket(temp_addr_info->ai_family, temp_addr_info->ai_socktype, temp_addr_info->ai_protocol);
        if (serverFD < 0) { 
            continue;
        }
        
        //gets rid of the "address already in use" error message	
        setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, (void*)true, sizeof(int));

        if (bind(serverFD, temp_addr_info->ai_addr, temp_addr_info->ai_addrlen) < 0){
            close(serverFD);
            continue;
        }
        break;
    }
	
	if (temp_addr_info == NULL) {	//port didn't get bound
		printf("problem with bind, choose a different port\n");
		return NULL;
    }
	printf("server initialized with port %s\n", port);
	
	freeaddrinfo(server);
	
	this->server = server;
	this->serverFD = serverFD;
	
	return this;
}

bool serv_send(UDPserver *a_srv, void* payload, int length, struct sockaddr dest){
	int addr_size = sizeof(dest);

	int buf_size = sendto(a_srv->serverFD, payload, length, 0, &dest, addr_size);
	if(buf_size < 0){
		perror("Error writing back to socket");
		return false;
	}
	
	return true;
}


int serv_recv(UDPserver *a_srv, void *payload, struct sockaddr* source){
	//memset(payload, 0, MAX_PACKET_SIZE);

	int fromlen  = sizeof *source;

	struct pollfd ufds;
	ufds.fd = a_srv->serverFD;
	ufds.events = POLLIN;

	unsigned int nfds = 1;
	int timeout = 2000, poll_rslt;
	
	if((poll_rslt = poll(&ufds, nfds, timeout) > 0)){
		int buf_size;
		
		buf_size = recvfrom(a_srv->serverFD, payload, MAX_PACKET_SIZE, 0, source, &fromlen);
		if (buf_size < 0)
		{
		     perror("Error reading from socket");
			return -1;
		}
		else{
			char ipstr[INET_ADDRSTRLEN];
			//printf("recv()'d %d bytes ", buf_size);
			//printf("from IP address %s\n", inet_ntop(AF_INET, source->sa_data, ipstr, sizeof ipstr));
		
			return buf_size;
		}
	}else 
		return poll_rslt;
}

bool serv_close(UDPserver* a_srv){
	close(a_srv->serverFD);
	free(a_srv);
	return true;
}

UDPclient* sock_n_conn(const char* src,  const char* port)
{
	struct addrinfo *server, settings;
	int serverFD, status;
	//char newport[5], *port = &newport;
	//itoa(portNum, port, 10);

	UDPclient *this = malloc(sizeof(UDPclient));

	memset(&settings, 0, sizeof settings);
	settings.ai_family = AF_INET;
    settings.ai_socktype = SOCK_DGRAM;

	if(status = (getaddrinfo(src, port, &settings, &server)) != 0){
		perror("Error getting address info");
		return NULL;
	}
	
	struct addrinfo *temp;
	for(temp = server; temp != NULL; temp = temp->ai_next) {
        if((serverFD = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) < 0){
			perror("Error creating socket");
			continue;
		}
		if(connect(serverFD,  temp->ai_addr, temp->ai_addrlen) < 0){
			close(serverFD);
			perror("Error connecting to server");
			continue;
		}
        break;
    }
	
	this->serverFD = serverFD;
	this->server = server;
	
	printf("Connection request sent on socket %d\n", serverFD);
	
	return this;
}

int cli_recv(UDPclient *a_client, uint8_t* payload){
	//memset(payload, 0, MAX_PACKET_SIZE);
	struct pollfd ufds;
	ufds.fd = a_client->serverFD;
	ufds.events = POLLIN;
	unsigned int nfds = 1;
	int timeout = 2000, poll_rslt;

	if((poll_rslt = poll(&ufds, nfds, timeout) > 0)){
		
		int buf_size;
		buf_size = recv(a_client->serverFD, payload, 255, 0);
		if (buf_size < 0)
		{
		     perror("Error reading from socket");
			return -1;
		}
		else{
			//printf("recv()'d %d bytes\n", buf_size);
			return buf_size;
		}
	}else 
		return poll_rslt;
}

bool cli_send(UDPclient *a_client, void *payload, int length){
	if(payload == NULL) return false;

	int buf_size = send(a_client->serverFD, payload, length, 0);

	if(buf_size < 0){
		perror("Error writing to socket");
		return false;
	}else
		return true;
}

bool cli_close(UDPclient *a_client){
	close(a_client->serverFD);
	free(a_client);
	return true;
}

/*
	RANDOM UTILITIES
*/



/*
int check_record(UDPserver* a_srv, struct sockaddr source){
	int i;
	for(i = 0; i < 10; i++){
		if(a_srv->clients[i]->clientAddr == source)
			return i;
		else if(a_srv->clients[i] == NULL){
			a_srv->clients[i]->clientAddr = source;
			return i;
		}	
	}
	printf("Received connection request from client, but server is full\n");
}
*/

/*
unsigned generateMessage(LSPMessage *msg, int conn_id, uint8_t* pld, int msg_len, int seq_num, void* buffer){

	unsigned response_size;
	msg->connid = conn_id;
	msg->seqnum = seq_num;
	msg->payload.data = malloc(sizeof(uint8_t) * msg_len);
	msg->payload.len = msg_len;
	if(msg_len > 0) memcpy(msg->payload.data, pld, msg_len * sizeof(uint8_t));
	response_size = lspmessage__get_packed_size(msg);
	
	buffer = malloc(response_size);
	lspmessage__pack(msg, buffer);
	
	return response_size;
}
*/
