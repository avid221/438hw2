#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <list>
#include <string>
#include <math.h>
#include "lsp_server.h"
#include <iostream>

using namespace std;


// Navigate through the range of passwords
// e.x. One run through increment takes aaa to aab
string increment(string input, int l){
    input[l -1]++;
    int i;
    for(i = l-1; i >= 0; i--){
        if (input[i]== '{') {
            input[i] = 'a';
            if(i-1 >= 0) {
                input[i-1]++;
            }
        }
    }
    return input;

}

// This is where the request is split into smaller increments
// Int z represents the amount of workers are able to work on this request
// Taking the range of the password and dividing it over the # of available workers
// An appropriate length is calculated called 'joblength'
// The function 'increment' is used to traverse the range of passwords
vector<string> split(string s1, string s2, int z) {

    vector<string> s;

    int length = pow(26, string(s1).length()) - 1;
    int joblength = ceil(length/z);
    string temp = s1;
    string end;
    for(int i=0; i<s1.length(); i++){
        end += "z";
    }

    s.push_back(s1);
    if(z == 1) {
        s.push_back(end);
        return s;
    }

    for(int a=0; a<z-1; a++) {
        //cout << "For each worker\n";
        for(int i=1; i<joblength; i++) {
            temp = increment(temp, string(temp).length());
        }
        //cout << "Next Range is " << temp << "\n";
        s.push_back(temp);
    }
    s.push_back(end);

    return s;
}

// A class that handles one request
class Assign {

public:

    string low, high, result, hash;
    uint32_t client;
    int length;
	vector<uint32_t> workers;

	// Request Creation
	Assign(const char * l, const char * h, const char * ha) {

		low = string(l);
		high = string(h);
        if( high.size() > low.size()) length = high.size();
        else length = low.size();
		length = max(high.size(), low.size());
		hash = string(ha);
		result = "";
	}
	// Is the worker is assigned to this task
	bool assigned(uint32_t w) {

		for(int i=0; i<workers.size(); i++) {

			if(workers[i] == w) return true;
        }

		return false;
	}
	
    // Destroy the worker running this task
	void kill(uint32_t w) {

		vector<uint32_t>::iterator it = workers.begin();

		while(it != workers.end()) {

			if (*it == w) it = workers.erase(it);
			else it++;
		}
	}
};

// Class for all server related functions
class Server {

public:

	lsp_server* connection;
    list<Assign> notbusy, busy;
    list<uint32_t> freeworkers;

    //Create Server
	Server(int port) {

		connection = lsp_server_create(port);

	}
    // Move to the next request
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

            vector<string> s = split(job.low, job.high, job.workers.size());

            for(int i=0; i<job.workers.size(); i++) {

                char msg[1024];
                memset(msg, 0, 1024);
                char c[2] = "c";
                char tok[] = " ";
                strcat(msg, c);
                strcat(msg, tok);
                strcat(msg, job.hash.c_str());
                strcat(msg, tok);
                strcat(msg, s[i].c_str());
                strcat(msg, tok);
                strcat(msg, s[i+1].c_str());
                unsigned int len = strlen((const char *)msg);
                cout << msg << "\n";
                if (!lsp_server_write(connection, (void *)msg, strlen((const char *)msg), job.workers[i])) {
                    cout << "There was a problem with the write to the worker.\n";
                } 
            }
            busy.push_back(job);
        }
    }
    // Is the worker currently 
    bool connected(uint32_t worker) {
        if (connection->clients[worker].conn_id == -1) return false;

        return true;
    } 
    // All the traffic is handled here
    void read(){

        uint32_t conn_id;
        char buf[1024];
        memset(buf, 0, 1024);
        int bytes = lsp_server_read(connection, buf, &conn_id);
        /*
        if (bytes == -1) {
            cout << "There was a problem with read.\n";
        }
        */
        if(bytes > 0) {
            char c = buf[0];
            if(c == 'j') {

                // Worker joins
                freeworkers.push_back(conn_id);
                printf("Worker %d joined.\n", conn_id);
            }
            else if(c == 'c') {

                // Request from Request
                char * hash, * high,* low;
                strtok(buf, " :");
                hash = strtok(NULL, " :");
                low = strtok(NULL, " :");
                high = strtok(NULL, "\n");
                Assign new_assignment(low, high, hash);
                new_assignment.client = conn_id;
                notbusy.push_back(new_assignment);
                printf("Request from client %d.\n", conn_id);
            }
            else if(c == 'f') {

                // Password found
                list<Assign>::iterator a = busy.begin();

                while(a != busy.end() && !a->assigned(conn_id)) {
                    a++;
                }

                if(a != busy.end()) {
                    if (!lsp_server_write(connection, (void *)buf, bytes, a->client)) {
                        cout << "There was a problem with the write to the request.\n";
                    }
                    strtok(buf, " :");
                    char * pass = strtok(NULL, "\n");
                    printf("Found: %s\n", pass);
                    a->kill(conn_id);
                    freeworkers.push_back(conn_id);
                    a->result = string(pass);
                }  
            }
            else if(c == 'x') {

                // Password not found
                list<Assign>::iterator a = busy.begin();

                while(a != busy.end() && !a->assigned(conn_id)) {
                    a++;
                }

                if(a != busy.end()) {
                    a->kill(conn_id);
                    if(busy.empty()) {
                        cout << "No more workers"
                    }
                    printf("Not Found\n");
                    if(!lsp_server_write(connection, (void *)buf, bytes, a->client)) {
                        cout << "There was a problem with the write to the request.\n";
                    }
                }
            }
        }
    }
    // Destroys Requests that have concluded
    void killdoneassigns() {

        list<Assign>::iterator it = busy.begin();

        while(it != busy.end()) {

            if(it->workers.size() == 0) {

                if(it->result == "") {

                    char *msg = "x";
                    if(!lsp_server_write(connection, (void *)msg, strlen((const char *)msg), it->client)) {
                        cout << "There was a problem with the write.\n";
                    }
                }

                it = busy.erase(it);
            }else {

                it++;
            }
        }
    }
    // Destroys Idle Workers
    void killdeadworkers(){

        // Navigate Workers
        list<uint32_t>::iterator w = freeworkers.begin();
        while(w != freeworkers.end()) {
            if(!connected(*w)) {
                //printf("Worker %d died.\n",*w);
                printf("Disonnected\n");
                if(!lsp_server_close(connection, *w)) {
                    cout << "There was a problem with closing the server.\n";
                }
                w = freeworkers.erase(w);
            }
            else {
                w++;
            }
        }

        // Navigate Assignments
        list<Assign>::iterator a = busy.begin();
        while(a != busy.end()) {
            vector<uint32_t>::iterator w = a->workers.begin();
            while(w != a->workers.end()) {
                if(!connected(*w)) {
                    //printf("Worker %d died.\n",*w);
                    printf("Disconnected\n");
                    if(!lsp_server_close(connection,*w)) {
                        cout << "There was a problem with closing the server.\n";
                    }
                    w = a->workers.erase(w);
                }
                else {
                    w++;
                }
            }
            a++;
        }

    }
};

int main(int argc, char ** argv) {
    
    srand(1234);
	int port = 7777;

	// If you don't like our port #
    if(argc > 1) port = atoi(argv[1]);

    Server served(port);
    
    bool keepgoing = true;

    while(keepgoing) {
        served.read();
        served.nextjob();
        served.killdeadworkers();
        served.killdoneassigns();
    }

    return 0;

        
}
