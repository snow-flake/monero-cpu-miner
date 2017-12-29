//
// Created by Snow Flake on 12/28/17.
//

#ifndef MONERO_CPU_MINER_DO_SKEIN_H
#define MONERO_CPU_MINER_DO_SKEIN_H

#include <cstring>

void do_skein_hash(const void* input, std::size_t len, char* output);

#endif //MONERO_CPU_MINER_DO_SKEIN_H
