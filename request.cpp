#include "lsp_client.h"
#include <stdlib.h>
#include <string>
#include <iostream>
//using namespace std;

lsp_client* gethostport(char* hostport,const char* host, char* port)
{
    std::string hp = std::string(hostport);
    std::string hosts = hp.substr(0,hp.length()-5);
    std::string ports = hp.substr(hp.length()-4);
	cout << "port = " << ports << '\n';
	cout << "host = " << hosts << '\n';
	host = hosts.c_str();
	
	int portint = atoi(ports.c_str());

	cout << "creating client at host " << hosts << "and port" << portint << "\n";
	lsp_client* myclient = lsp_client_create(host, portint); //creates local client
	return myclient;
}

int makemsg(char* hash, int length,char* msg)
{

	char begin[10];
	char end[10];
	int msglen = 26;
	memset(begin,0,10);
	memset(end,0,10);
	std::string a = "a";
	std::string z = "z";
	std::string sp = " ";
	cout << "here";
	strcat(begin,sp.c_str());
	strcat(end,sp.c_str());
	for (int i = 0; i < length; ++i)
	{
		strcat(begin,a.c_str());
		strcat(end,z.c_str());
		msglen++;
	}
	//strcpy(msg,"c ");
	//strcat(msg,hash);
	//strcat(msg,begin);
	//strcat(msg,end);
	std::string msgs = "c " + hash + begin + end;
	cout << "message: " << msgs << '\n' << "msglen = " << msglen << '\n';
	return msglen;
}

void read(lsp_client* myclient)
{
		int read;
	uint8_t buf[7];
	while (true)
	{
		if (((lsp_client_read(myclient,buf) > 0) && ((buf[0]=='x') || (buf[0]=='f')))) break;
	}
	char* pwdmsg;
	if (buf[0]=='f'){
		strcpy(pwdmsg,"Found:");
		strcat(pwdmsg,strchr((char*)buf,' '));
	}
	else if (buf[0] == 'x')
		strcpy(pwdmsg,"Not Found");
	printf(pwdmsg);
	printf("%i",buf); //print password
}

int main(int argc, char** argv) 
{
	
	char* host;
	char* port;
	char* msg;
	char* a2 = argv[2];
	char* a3 = argv[3];
	
	lsp_client* myclient = gethostport(argv[1],host,port);
	cout << "gothostport\n";
	cout << a2 << '\n' << a3 << '\n';
	int len = atoi(a3);
	int msglen = makemsg(a2,len,msg);
	cout << "writing\n";
	lsp_client_write(myclient,(uint8_t*)msg,strlen(msg)); //send HASH to be cracked
	cout << "reading\n";
	read(myclient);
		
	lsp_client_close(myclient); //close client
}
