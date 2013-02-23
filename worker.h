#include <iostream>
#include <vector>
#include <string>
#include <openssl/sha.h>
#include <sstream>

class Worker{
public:
	Worker(const char* dest, int port);

private:
	std::vector<std::string> combos(std::string lower, std::string upper);
};
