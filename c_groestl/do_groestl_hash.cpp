//
// Created by Snow Flake on 12/28/17.
//

#include "do_groestl_hash.hpp"
#include "c_groestl.hpp"

void do_groestl_hash(const void* input, std::size_t len, char* output) {
	c_groestl::groestl((const uint8_t*)input, len * 8, (uint8_t*)output);
}
