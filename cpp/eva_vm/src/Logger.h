#ifndef Logger_h
#define Logger_h

#include <sstream>

class ErrorLogMessage : public std::basic_ostringstream<char> {
public:
	~ErrorLogMessage() {
		fprintf(stderr, "Error: %s\n", str().c_str());
		exit(EXIT_FAILURE);
	}
};

#define DIE ErrorLogMessage()
#define log(value) std::cout << #value << " = " << (value) << "\n";

#endif