#include <iostream>
#include <vector>
#include <string>
#include <openssl/sha.h>

class Worker{
public:
	Worker(const char* dest, int port, uint8_t* payload, int length, int read, uint8_t* hash);

private:
	std::vector<std::string> combos(int length);
};
