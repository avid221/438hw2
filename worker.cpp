#include "lsp_client.h"
#include "worker.h"

using namespace std;

Worker::Worker(const char* dest, int port){
	lsp_client* client = lsp_client_create(dest, port);  //Create client
	if(lsp_client_write(client, (uint8_t*)"j", strlen((const char*)"j"))){  //request a join from client
		cout << "connected" << endl;
	}
	else{cout << "no response from server" << endl;}
	unsigned char* result = (unsigned char*)malloc(1024);
	uint8_t* payload = (uint8_t*)malloc(1024);
	int readval = 0;
	while(true){
		readval = lsp_client_read(client, payload); //wait to get a request from the server
		if(readval > 0){  //retrieves target hash from server
			string plds = string((char*)payload);
			string hashs = plds.substr(2,40);
			
			int len = (plds.length()-44)/2;
			string lower = plds.substr(43, len);
			string upper = plds.substr(43+len+1, len);
			
			stringstream str;
			vector<string> possible = combos(lower, upper);
			/*
			This loop cycles through all possible combinations and if it finds the correct hash
			it send the success message to the server in the form of "f pass" where 'pass' is the cracked password
			*/
			for(int i = 0; i < possible.size(); i++){ 
				SHA1((const unsigned char*)possible[i].c_str(), (unsigned long)len, result);
				char* real;
				for(int j = 0; j < 20; j++){
					sprintf(real+j*2, "%02x", result[j]);
				}
				string reals = string(real);
				if(hashs == reals){
					string raw_success = "f";
					raw_success += " " + possible[i];
					
					uint8_t* success = (uint8_t*)malloc(sizeof(uint8_t*) * raw_success.length());
					strcpy((char*)success, raw_success.c_str());\
					printf("%s\n", success);
					
					if(lsp_client_write(client, (uint8_t*)success, strlen((const char*)success))){
						cout << "success sent" << endl;
					}
					else{cout << "sending failed" << endl;}
					
					free(success);
					break;
				}
				else if(i == possible.size()-1){
					if(lsp_client_write(client, (uint8_t*)"x", strlen((const char*)"x"))){
						cout << "Notice of failure sent" << endl;
					}
					else{cout << "Notice of failure failed to send" << endl;}
				}
			}
		}
	}
	free(result);
	free(payload);
}

/*
This is the part of the worker that actually makes the password cracking possible.
It cycles through the given range of letters finding every combination within that range.
It then returns a vector of these possibilties.
*/
vector<string> Worker::combos(string lower, string upper){
	int length = lower.length();
	vector<int> index(length, 0);
	char alpha[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 
					'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
					'w', 'x', 'y', 'z'};
					
	for(int i = 0; i < length; i++){
		for(int j = 0; j < sizeof(alpha); j++){
			if(lower[i] == alpha[j]){
				index[i] = j;
				break;
			}
		}
	}
	
	vector<string> passes;
	
	while(true){
		string pass;
		
		for(int i = 0; i < length; i++){
			pass += alpha[index[i]];
		}
		
		passes.push_back(pass);
		
		if(passes[passes.size()-1] == upper){
				return passes;
		}
		
		for(int i = length-1;; i--){
			if(i < 0){
				return passes;
			}
			
			index[i]++;
			
			if(index[i] == sizeof(alpha)){
				index[i] = 0;
			}
			else{
				break;
			}
		}
	}
}

/*
makes the worker run upon creation

TO RUN:
./worker host:port	(i.e. ./worker 127.0.0.1:7777)
*/
int main(char argc, char** argv){
	const char* dest = "127.0.0.1";
	srand(12345);
	string argvs = string(argv[1]);
	string dests = argvs.substr(0,argvs.length()-5);
	string ports = argvs.substr(argvs.length()-4,4);

	int port = atoi(ports.c_str());
	dest = dests.c_str();
	
	Worker worker(dest, port);
}

