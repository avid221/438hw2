#include "lsp_client.h"
#include <stdlib.h>
#include <string>
#include <iostream>
//using namespace std;

lsp_client* gethostport(char* hostport,const char* host, char* port)
{
    std::string hp = std::string(hostport); //parse host and port from first argument
    std::string hosts = hp.substr(0,hp.length()-5);
    std::string ports = hp.substr(hp.length()-4);
	host = hosts.c_str();
	
	int portint = atoi(ports.c_str());

	lsp_client* myclient = lsp_client_create(host, portint); //creates local client
	return myclient;
}

void makemsg(char* hash, int length,char* msg, lsp_client* myclient) //make and send message
{

	int msglen = 26; //will increment with begin and end screens
	std::string sp = " ";
	std::string begin = sp;
	std::string end = sp; 
	for (int i = 0; i < length; ++i)
	{
		begin = begin + 'a';
		end = end + 'z';
		msglen++;
	}
	std::string hashs = std::string(hash);
	std::string msgs = "c " + hashs + begin + end;

	msg = (char*)msgs.c_str();
	lsp_client_write(myclient,(uint8_t*)msg,msgs.length()); //send HASH to be cracked
	
}

void read(lsp_client* myclient)
{
		int read=0;
	uint8_t buf[1024];
	buf[0] = 0;
	while (true) //read until a relevant message comes back
	{

		if (lsp_client_read(myclient,buf) > 0)
			 if ((buf[0]=='x') || (buf[0]=='f')) break;

	}
	std::string pwdstr,pwd;
	char* pwdmsg;
	if (buf[0]=='f'){ //found case
		pwd = std::string((char*)buf);
		
		pwdstr = "Found: " + pwd.substr(2);
	}
	else if (buf[0] == 'x') //not found case
		pwdstr = "Not Found.";
	else {} //disconnect case
}

int main(int argc, char** argv) 
{
	
	char* host;
	char* port;
	char* msg;
	char* a2 = argv[2]; //read arguments
	char* a3 = argv[3];
	
	lsp_client* myclient = gethostport(argv[1],host,port);

	int len = atoi(a3);
	makemsg(a2,len,msg,myclient);
	read(myclient);
		
	lsp_client_close(myclient); //close client
}



