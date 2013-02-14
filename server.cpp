#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <list>
#include <string>
#include <math.h>
#include "lsp.h"

using namespace std;


// Helper Business
int max(int x, int y) { 

	if (x > y) return x;
	else return y;
}

int toint(string s) {

    int alpha = 26;
    int result = 0;
    string temp;

    // Reverse String
    for(int i=s.size()-1; i>=0; i--) {
        temp.push_back(s[i]);
    }

    for(int i=0; i<temp.size(); i++) {

        int digit = ( pow(alpha, i) * (temp[i]) ) - 'a';
        result+= digit;
    }

    return result;
}

string tostring(int i) {

    char* alphabet = "abcdefghijklmnopqrstuvwxyz";
    int alpha = 26;
    string s;

    if(i < alpha) {
        s.push_back(alphabet[i]);
        return s;
    }

    s = tostring(i/alpha);
    s.push_back(alphabet[i%alpha]);

    return s;
}

vector<string> split(const char* s1, const char* s2, int z) {

    vector<string> s;

    int a = toint( string(s1) );
    int b = toint( string(s2) );
    int temp = (a + b) / z;

    s.push_back(string(s1));

    for(int i=1; i<z; i++) {

            s.push_back( tostring(temp*i) );
    }

    s.push_back(string(s2));

    return s;
}

// These things don't like each other unless you use uint32_t
class Assign {

public:

    string low, high, result, hash;
    uint32_t client;
    int length;
	vector<uint32_t> workers;
		
	Assign(const char * l, const char * h, const char * ha) {

		low = string(l);
		high = string(h);
		length = max(high.size(), low.size());
		hash = string(ha);
		result = "";
	}
		
	bool assigned(uint32_t w) {

		for(int i=0; i<workers.size(); i++) {

			if(workers[i] == w) return true;
        }

		return false;
	}
	
	void kill(uint32_t w) {

		vector<uint32_t>::iterator it = workers.begin();

		while(it != workers.end()) {

			if (*it == w) it = workers.erase(it);
			else it++;
		}
	}
};

class Server {

public:

	lsp_server* connection;
    list<Assign> notbusy, busy;
    list<uint32_t> freeworkers;

	// lsp_server* connection; this thing doesn't exist BUT I HOPE IT DOES
	// Make dat server baybay, better be int Alek
	Server(int port) {

		connection = lsp_server_create(port);

	}

    void nextjob() {

        if(freeworkers.size() > 0 && notbusy.size() > 0) {

            // Pull worker from queue
            Assign job = notbusy.front();
            notbusy.pop_front();  
            
            // Assign workers
            list<uint32_t>::iterator it = freeworkers.begin();

            while(it != freeworkers.end()) {

                job.workers.push_back(*it);
                it = freeworkers.erase(it);
            }

            vector<string> s = split(job.high.c_str(), job.low.c_str(), job.workers.size());

            for(int i=0; i<job.workers.size(); i++) {

                char msg[66000];
                memset(msg, 0, 66000);
                char tok[] = ":";
                strcat(msg, job.hash.c_str());
                strcat(msg, tok);
                strcat(msg, s[i].c_str());
                strcat(msg, tok);
                strcat(msg, s[i+1].c_str());
                unsigned int len = strlen(msg);     //unsigned baby
                lsp_server_write(connection, (uint8_t *)msg, strlen((const char *)msg), job.workers[i]);
            }
            busy.push_back(job);
        }
    }

    bool connected() {} //how is the client handling this
    void read(){}
    void killdoneassigns() {

        list<Assign>::iterator it = busy.begin();

        while(it != busy.end()) {

            if(it->workers.size() == 0) {

                if(it->result == "") {

                    char *msg = "x";
                    lsp_server_write(connection, msg, strlen(msg), it->client);
                }

                it = busy.erase(it);
            }else {

                it++;
            }
        }
    }
    void killdeadworkers(){}
};

int main(int argc, char ** argv) {

		int port = 7777;

		// If you don't like our port #
        if(argc > 1) port = atoi(argv[1]);

        printf("Prepare for big plays\n");
		
        Server served(port);
        
        bool keepgoing = true;

		//this running forever
        while(keepgoing) {
			//Read Server
			//Assign a Job
			//Destroy Disconnected Workers
			//Destroy Finished Assignments

        }
        
        return 0;

        
}
