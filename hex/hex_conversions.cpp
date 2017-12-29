//
// Created by Snow Flake on 12/29/17.
//

#include "hex_conversions.h"
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

namespace hex {

	const std::vector<unsigned char> decodeHex(const std::string &source) {
		std::string hexCode;
		CryptoPP::StringSource(source, true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(hexCode)));

		std::vector<unsigned char> output = std::vector<unsigned char>(hexCode.size());
		for (std::string::iterator iter = hexCode.begin(); iter < hexCode.end(); iter++) {
			const char c = *iter;
			output.push_back((unsigned char )c);
		}

		return output;
	}

	const std::string encodeHex(const std::vector<unsigned char> &source) {
		std::string output;
		CryptoPP::StringSource ss(&source[0], source.size(), true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(output)));
		return output;
	}


	inline unsigned char hf_hex2bin(char c, bool &err) {
		if (c >= '0' && c <= '9')
			return c - '0';
		else if (c >= 'a' && c <= 'f')
			return c - 'a' + 0xA;
		else if (c >= 'A' && c <= 'F')
			return c - 'A' + 0xA;

		err = true;
		return 0;
	}

	inline char hf_bin2hex(unsigned char c) {
		if (c <= 0x9)
			return '0' + c;
		else
			return 'a' - 0xA + c;
	}

	bool hex2bin(const std::string &in, unsigned char *out) {
		return hex2bin(in.c_str(), in.size(), out);
	}

	bool hex2bin(const char *in, unsigned int len, unsigned char *out) {
		bool error = false;
		for (unsigned int i = 0; i < len; i += 2) {
			out[i / 2] = (hf_hex2bin(in[i], error) << 4) | hf_hex2bin(in[i + 1], error);
			if (error) return false;
		}
		return true;
	}

	void bin2hex(const std::string &in, char *out) {
		bin2hex((const unsigned char *) in.c_str(), in.size(), out);
	}

	void bin2hex(const unsigned char *in, unsigned int len, char *out) {
		for (unsigned int i = 0; i < len; i++) {
			out[i * 2] = hf_bin2hex((in[i] & 0xF0) >> 4);
			out[i * 2 + 1] = hf_bin2hex(in[i] & 0x0F);
		}
	}
}