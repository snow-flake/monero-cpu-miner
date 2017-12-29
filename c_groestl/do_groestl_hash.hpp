//
// Created by Snow Flake on 12/28/17.
//

#ifndef MONERO_CPU_MINER_GROESTL_HASH_H
#define MONERO_CPU_MINER_GROESTL_HASH_H

#include <cstring>

void do_groestl_hash(const void* input, std::size_t len, char* output);

#endif //MONERO_CPU_MINER_GROESTL_HASH_H
