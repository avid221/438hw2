#include <iostream>
#include <vector>
#include <string>
#include <openssl/sha.h>

class Worker{
public:
	Worker(const char* dest, int port);

private:
	std::vector<std::string> combos(int length);
};
