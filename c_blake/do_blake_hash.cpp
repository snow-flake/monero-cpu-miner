//
// Created by Snow Flake on 12/28/17.
//

#include "c_blake256.h"
#include "do_blake_hash.hpp"

void do_blake_hash(const void* input, std::size_t len, char* output) {
	c_blake::blake256_hash((uint8_t*)output, (const uint8_t*)input, len);
}
