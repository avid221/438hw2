#include "lsp_client.h"


int main(int argc, char** argv) 
{
	int port = 7777; //test default port
	if (atoi(argv[1])>1) port = atoi(argv[1]); //inputs port if input is given
	lsp_client* myclient = lsp_client_create("127.0.0.1", port); //creates local client
	lsp_client_write(myclient,pld,length); //send password request
	int read;
	uint8_t  buf;
	while (read > 0) //until successful read
	{
		read = lsp_client_read(myclient,&buf);	//get password
	}
	printf("%i",buf); //print password
		
	lsp_client_close(myclient); //close client
}
