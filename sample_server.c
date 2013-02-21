#include "lsp_server.h"

int main(int argc, char** argv) 
{
	lsp_server* myserver;

	if(argc != 2){
		printf("Usage: %s <port>\n", atoi(argv[0]));
		exit(0);
	}else
		myserver = lsp_server_create(atoi(argv[1]));
	
	
	uint8_t payload[MAX_PACKET_SIZE];
	uint32_t returned_id;
	int bytes_read;
	
	int counter = 0;
	
	while(counter < 5)
	{		
		printf("Listening for packets...\n");
		int bytes = lsp_server_read(myserver, &payload, &returned_id);

		//	Echo it right back if we got something
		if(bytes > 0){
			printf("from conn. id %d: %s\n", returned_id, payload);
			lsp_server_write(myserver, payload, bytes, returned_id);
			counter++;
		}
		
	//	if(counter > 3) lsp_server_close(myserver, 1);
	}
	
	return 0;
}
