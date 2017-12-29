//
// Created by Snow Flake on 12/29/17.
//

#ifndef MONERO_CPU_MINER_DO_CRYPTONIGHT_H
#define MONERO_CPU_MINER_DO_CRYPTONIGHT_H

#include <cstdint>

enum memory_mode_t {
	never_use = 1,
	no_mlck = 2,
	print_warning = 3,
	always_use = 4,
};

enum currency_mode_t {
	currency_monero = 1,
	currency_aeon = 0,
};

enum aes_mode_t {
	aes_enabled = 1 << 2,
	aes_disabled = 0 << 2,
};

enum prefetch_mode_t {
	prefetch_enabled = 1 << 4,
	prefetch_disabled = 0 << 4,
};

enum hash_mode_t {
	currency_monero__aes_disabled__prefetch_disabled = 1,
	currency_monero__aes_disabled__prefetch_enabled = 2,
	currency_monero__aes_enabled__prefetch_disabled = 3,
	currency_monero__aes_enabled__prefetch_enabled = 4,
	currency_aeon__aes_disabled__prefetch_disabled = 5,
	currency_aeon__aes_disabled__prefetch_enabled = 6,
	currency_aeon__aes_enabled__prefetch_disabled = 7,
	currency_aeon__aes_enabled__prefetch_enabled = 8,
};

struct CryptonightWork {
	memory_mode_t memory_mode;
	hash_mode_t hash_mode;
	uint8_t input[112];
	uint32_t input_size;
	uint64_t output_target;
	uint8_t output[32];

};

bool do_cryptonight(CryptonightWork & work);

#endif //MONERO_CPU_MINER_DO_CRYPTONIGHT_H
