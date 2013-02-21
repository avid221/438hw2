#include "lsp_client.h"
#include "worker.h"

using namespace std;
/*

TODO:
finagle with adding words to a uin8_t*

*/
Worker::Worker(const char* dest, int port, uint8_t* payload, int length, int read, uint8_t* hash){
	lsp_client* client = lsp_client_create(dest, port);
	lsp_client_write(client, payload, length);
		
	while(true){
		lsp_client_read(client, payload);
		length = sizeof(payload);

		vector<string> possible = combos(sizeof(payload));
		for(int i = 0; i < possible.size(); i++){
			unsigned char* result = SHA1((const unsigned char*)possible[i].c_str(), (unsigned long)sizeof(possible[i]), (unsigned char*)hash);

			if(result == payload){
				uint8_t* success = (uint8_t*)"SUCCESS: ";
				//success += possible[i];
				lsp_client_write(client, success, length);
				break;
			}
			else if(i == possible.size()-1){
				uint8_t* failure = (uint8_t*)"FAILURE";
				lsp_client_write(client, failure, length);
			}
		}
	}
}

vector<string> Worker::combos(int length){
	vector<int> index(length, 0);
	vector<string> passes;
	char alpha[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 
					'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
					'w', 'x', 'y', 'z'};
	
	while(true){
		string pass;
		
		for(int i = 0; i < length; i++){
			pass += alpha[index[i]];
		}

		//cout << pass << endl;
		passes.push_back(pass);
		
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


