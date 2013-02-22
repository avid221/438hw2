#include "lsp_client.h"

void gethostport(char* hostport,char* host, char* port)
{
    strtok(hostport, ":");
    host = strtok(NULL, ":");
    port = strtok(NULL, " ");
	fprintf("port = " + port);
	fprintf("host = " + host);
}

int makemsg(char* hash, int length,char* msg)
{
	char* begin;
	char* end;
	int msglen = 26;
	strcpy(begin,'a');
	strcpy(end,'z');
	for (int i = 1; i < length; ++i)
	{
		strcat(begin,'a');
		strcat(end,'z');
		msglen++;
	}
	strcpy(msg,"c ");
	strcat(msg,hash);
	strcat(msg,begin);
	strcat(msg,end);
	return msglen;
}

int main(int argc, char** argv) 
{
	char* hostport = argv[1];
	char* host;
	char* port;
	char* msg;
	gethostport(hostport,host,port);
	int msglen = makemsg(argv[2],argv[3],msg)
	lsp_client* myclient = lsp_client_create(host, port); //creates local client
	lsp_client_write(myclient,msg,msglen); //send HASH to be cracked
	int read;
	uint8_t buf[7];
	while (buf[0] != 'f' && buf[0] != 'x') //until successful read
	{
		if buf[0] != NULL
		read = lsp_client_read(myclient,&buf);	//get password
	}
	char* pwdmsg;
	if (buf[0]=='f'){
		strcpy(pwdmsg,"Found:");
		strcat(pwdmsg,strchr(buf,' '));
	}
	else if (buf[0] == 'x')
		strcpy(pwdmsg,"Not Found");
	printf(pwdmsg);
	printf("%i",buf); //print password
		
	lsp_client_close(myclient); //close client
}
