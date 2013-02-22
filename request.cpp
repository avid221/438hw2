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
	cout << "port = " << ports << '\n';
	cout << "host = " << hosts << '\n';
	host = hosts.c_str();
	
	int portint = atoi(ports.c_str());

	cout << "creating client at host " << hosts << ":" << portint << "\n";
	lsp_client* myclient = lsp_client_create(host, portint); //creates local client
	return myclient;
}

void makemsg(char* hash, int length,char* msg, lsp_client* myclient)
{

	int msglen = 26;
	std::string sp = " ";
	//strcat(begin,sp.c_str());
	//strcat(end,sp.c_str());
	std::string begin = sp;
	std::string end = sp;
	for (int i = 0; i < length; ++i)
	{
		begin = begin + 'a';
		end = end + 'z';
		msglen++;
	}
	std::string hashs = std::string(hash);
	//strcpy(msg,"c ");
	//strcat(msg,hash);
	//strcat(msg,begin);
	//strcat(msg,end);
	std::string msgs = "c " + hashs + begin + end;
	cout << "writing "<< msgs << '\n';
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
		cout << "preparing to read\n";
		if (lsp_client_read(myclient,buf) > 0)
			 if ((buf[0]=='x') || (buf[0]=='f')) break;
		cout <<"read " << buf[0] << '\n';
	}
	std::string pwdstr,pwd;
	char* pwdmsg;
	if (buf[0]=='f'){ //found case
		pwd = std::string((char*)buf);
		//strcat(pwdmsg,strcatchr((char*)buf,' '));
		pwdstr = "Found: " + pwd.substr(2);
	}
	else if (buf[0] == 'x') //not found case
		pwdstr = "Not Found.";
	else {} //disconnect case
	cout << pwdstr << '\n';  //print password
}

int main(int argc, char** argv) 
{
	
	char* host;
	char* port;
	char* msg;
	char* a2 = argv[2];
	char* a3 = argv[3];
	
	lsp_client* myclient = gethostport(argv[1],host,port);
	cout << a2 << '\n' << a3 << '\n';
	int len = atoi(a3);
	makemsg(a2,len,msg,myclient);
	read(myclient);
		
	lsp_client_close(myclient); //close client
}
