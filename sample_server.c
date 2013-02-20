#include "lsp_server.h"

int main(int argc, char** argv) 
{
	lsp_server* myserver;

	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		exit(0);
	}else
		myserver = lsp_server_create(argv[1]);
	
	
	uint8_t payload[1024];
	uint32_t returned_id;
	int bytes_read;
	
	int counter = 0;

	for(;;)
	{
		sleep(1);
		
		printf("Listening for packets...\n");
		int bytes = lsp_server_read(myserver, &payload, &returned_id);

		//	Echo it right back if we got something
		if(bytes > 0){
			printf("from conn. id %d %s\n", returned_id, payload);
			lsp_server_write(myserver, payload, bytes, returned_id);
			counter++;
		}
	}
	lsp_server_close(myserver, 0);
	return 0;
}
