//
// Created by Snow Flake on 12/28/17.
//

#ifndef MONERO_CPU_MINER_DO_KECCAK_H
#define MONERO_CPU_MINER_DO_KECCAK_H

#include <stdint.h>

// compute a keccak hash (md) of given byte length from "in"
int do_keccak(const uint8_t *in, int inlen, uint8_t *md, int mdlen);
// update the state
void do_keccakf(uint64_t st[25], int norounds);

#endif //MONERO_CPU_MINER_DO_KECCAK_H
