#include "logger/logger.hpp"
#include "c_cryptonight/do_cryptonight.hpp"

int main() {
	logger::log_success(__FILE__, __LINE__, "Hello world!");

	const std::string input = "0606d6ef8ed205ac3480614a522658372bd7974f83e9b1ec8c744ec3f9a0ab0b9a6de14942527d00000000bdd1db8d3e1755149156dbd70e648f12cf305456bda91caf7282f92dacb96d7715";
	const std::string output = "7a4ed9aed9937774a78ca3dbb7e9f8f47f102a0bd5c411047d051a744d001212";

	CryptonightWork work = CryptonightWork(always_use, currency_monero__aes_disabled__prefetch_disabled, input);

	logger::log_success(__FILE__, __LINE__, "Expected input:");
	logger::log_success(__FILE__, __LINE__, input);
	logger::log_success(__FILE__, __LINE__, "Expected output:");
	logger::log_success(__FILE__, __LINE__, output);

	logger::log_success(__FILE__, __LINE__, "Actual input:");
	logger::log_success(__FILE__, __LINE__, work.input_str());
	do_cryptonight(work);
	logger::log_success(__FILE__, __LINE__, "Actual output:");
	logger::log_success(__FILE__, __LINE__, work.output_str());


//	bResult = memcmp(out, "\xa0\x84\xf0\x1d\x14\x37\xa0\x9c\x69\x85\x40\x1b\x60\xd4\x35\x54\xae\x10\x58\x02\xc5\xf5\xd8\xa9\xb3\x25\x36\x49\xc0\xbe\x66\x05", 32) == 0;

	logger::log_success(__FILE__, __LINE__, "Goodbye world!");
	return 0;
}
