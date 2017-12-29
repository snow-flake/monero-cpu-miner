#include "logger/logger.hpp"
#include "c_cryptonight/do_cryptonight.hpp"

int main() {
	logger::log_success(__FILE__, __LINE__, "Hello world!");
	logger::log_success(__FILE__, __LINE__, "Goodbye world!");
	return 0;
}
