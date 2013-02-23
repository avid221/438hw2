#include "lsp_client.h"
#include "worker.h"

using namespace std;
/*

TODO:
finagle with adding words to a uin8_t*

*/
Worker::Worker(const char* dest, int port){
	lsp_client* client = lsp_client_create(dest, port);
	lsp_client_write(client, (uint8_t*)"j", strlen((const char*)"j"));
	
	unsigned char* result = (unsigned char*)malloc(1024);
	uint8_t* payload = (uint8_t*)malloc(1024);
	int readval = 0;
	while(true){
		cout << "starting read statement\n";
		readval = lsp_client_read(client, payload);
		if(readval > 0){  //retrieves target hash from server
			cout << "received packet of size " << readval << '\n';
			
			string plds = string((char*)payload);
			string hashs = plds.substr(2,40);
			cout << hashs << '\n';
			
			int len = (plds.length()-44)/2;
			string lower = plds.substr(42, len);
			string upper = plds.substr(42+len+1, len);
			
			vector<string> possible = combos(lower, upper);
			cout << "possible size " << possible.size() << '\n';
			for(int i = 0; i < possible.size(); i++){
				//cout << "getting sha on" << endl;
				SHA1((const unsigned char*)possible[i].c_str(), (unsigned long)sizeof(possible[i]), result);
				string results = string((char*)result);
				//printf("%d\n",atoi((const char*)result[0]));
				if(result == (unsigned char*)hashs.c_str()){
					string raw_success = "f";
					raw_success += " " + possible[i];
					
					uint8_t* success = (uint8_t*)malloc(sizeof(uint8_t*) * raw_success.length());
					strcpy((char*)success, raw_success.c_str());
					
					lsp_client_write(client, (uint8_t*)success, strlen((const char*)success));
					
					free(success);
					break;
				}
				else if(i == possible.size()-1){
					lsp_client_write(client, (uint8_t*)"x", strlen((const char*)"x"));
				}
			}
		}
		else if(readval == -1){
			cout << "waiting" << endl;
		}
		else cout << "got " << readval << " sized packet\n";
	}
	free(result);
	free(payload);
}

vector<string> Worker::combos(string lower, string upper){
	//cout << "combos len " << length << '\n';
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
		
		//cout << pass << endl;
		passes.push_back(pass);
		
		for(int i = length-1;; i--){
			if(passes[passes.size()-1] == upper){
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

int main(char argc, char** argv){
	const char* dest = "127.0.0.1";
	string argvs = string(argv[1]);
	string dests = argvs.substr(0,argvs.length()-5);
	string ports = argvs.substr(argvs.length()-4,4);

	int port = atoi(ports.c_str());
	dest = dests.c_str();
	
	Worker worker(dest, port);
}

