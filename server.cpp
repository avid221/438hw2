#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <list>
#include <string>
#include <math.h>
#include "lsp_server.h"
#include <iostream>

using namespace std;


// Helper Business
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

unsigned long long convert(string str)
{
    unsigned long long result = 0;
    for (int i = 0;i<str.length();i++)
        result+= (str[i] - 'A' + 1) + i*26;
    return result;
}

int frombase(string s) {
    int temp = 0;
    if(s != "" && s.length() > 0) {
        temp = s[0] - 'A';
        for (int i=1; i<s.length(); i++) {
            temp *= 26;
            temp += s[i] - 'A';
        }
    }
    return temp;
}
string tobase(int n) {
    int number = abs(number);
    string converted = "";

    while (number > 0) {
        int rem = number%26;
        converted = (char)(rem + 'A') + converted;
        number = (number - rem) / 26;

    }
    return converted;

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
    cout << "Current letter is " << s << "\n";
    s.push_back(alphabet[i%alpha]);

    return s;
}

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

    for(int a=0; a<z; a++) {
        //cout << "For each worker\n";
        for(int i=1; i<joblength; i++) {
            temp = increment(temp, string(temp).length());
        }
        cout << "Next Range is " << temp << "\n";
        s.push_back(temp);
        if (a=z) {
            s.push_back(end);
            break;
        }
    }
    for (int i=0; i<s.size(); i++){
        cout << s[i] << "\n";
    }

    /*
    int a = toint( string(s1) );
    int b = toint( string(s2) );
    int temp = (a + b) / z;

    s.push_back(string(s1));

    for(int i=0; i<z; i++) {

        s.push_back( tostring(temp*i) );
    }

    s.push_back(string(s2));
    */
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
        if( high.size() > low.size()) length = high.size();
        else length = low.size();
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

            vector<string> s = split(job.low, job.high, job.workers.size());

            for(int i=0; i<job.workers.size(); i++) {

                char msg[1024];
                memset(msg, 0, 1024); //sends hash:start:end
                char c[2] = "c";
                char tok[] = " ";
                strcat(msg, c);
                strcat(msg, tok);
                strcat(msg, job.hash.c_str());
                strcat(msg, tok);
                strcat(msg, s[i].c_str());
                strcat(msg, tok);
                strcat(msg, s[i+1].c_str());
                unsigned int len = strlen(msg);     //unsigned baby
                //printf("%d\n",strlen((const char*)msg));
                string msgs = string( (char *)msg);
                cout << msgs;
                lsp_server_write(connection, (uint8_t *)msg, strlen((const char *)msg), job.workers[i]);
            }
            busy.push_back(job);
        }
    }
    //*** This shit needs work***
    bool connected(uint32_t worker) {
        if (connection->clients[worker].conn_id == -1) return false;

        return true;

    } //how is the client handling this
    void read(){

        uint32_t conn_id;
        char buf[1024];
        memset(buf, 0, 1024);
        int bytes = lsp_server_read(connection, buf, &conn_id);
        
        if(bytes > 0) {
            char c = buf[0];
            if(c == 'j') {

                // Worker joins
                freeworkers.push_back(conn_id);
                printf("Worker %d joined.\n", conn_id);
            }
            else if(c == 'c') {
                //printf(buf);
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
                    lsp_server_write(connection, buf, bytes, a->client);
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
                    printf("Not Found\n");
                    lsp_server_write(connection, buf, bytes, a->client);
                }
            }
        }
    }
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
    void killdeadworkers(){

        // Navigate Workers
        list<uint32_t>::iterator w = freeworkers.begin();
        while(w != freeworkers.end()) {
            if(!connected(*w)) {
                //printf("Worker %d died.\n",*w);
                printf("Disonnected");
                lsp_server_close(connection, *w);
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
                    printf("Disconnected");
                    lsp_server_close(connection,*w);
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

    printf("Prepare for big plays\n");
	
    Server served(port);
    
    bool keepgoing = true;

	//this running forever
    while(keepgoing) {
        served.read();
        served.nextjob();
        served.killdeadworkers();
        served.killdoneassigns();
		//Read Server
		//Assign a Job
		//Destroy Disconnected Workers
		//Destroy Finished Assignments

    }
    return 0;

        
}
