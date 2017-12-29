//
// Created by Snow Flake on 12/29/17.
//

#ifndef MONERO_CPU_MINER_HEX_CONVERSIONS_H
#define MONERO_CPU_MINER_HEX_CONVERSIONS_H

#include <string>
#include <vector>

namespace hex {
	const std::vector<unsigned char> decodeHex(const std::string & source);
	const std::string encodeHex(const std::vector<unsigned char> &source);

	bool hex2bin(const std::string & in, unsigned char * out);
	bool hex2bin(const char* in, unsigned int len, unsigned char* out);

	void bin2hex(const std::string & in, char* out);
	void bin2hex(const unsigned char* in, unsigned int len, char* out);
}

#endif //MONERO_CPU_MINER_HEX_CONVERSIONS_H
