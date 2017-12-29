//
// Created by Snow Flake on 12/28/17.
//

#include "do_jh_hash.hpp"
#include "c_jh.hpp"

void do_jh_hash(const void* input, std::size_t len, char* output) {
	c_jh::jh_hash(32 * 8, (const uint8_t*)input, 8 * len, (uint8_t*)output);
}
