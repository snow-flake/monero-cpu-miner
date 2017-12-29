#ifndef C_CRYPTONIGHT_CRYPTONIGHT_H
#define C_CRYPTONIGHT_CRYPTONIGHT_H

#include <stddef.h>
#include <inttypes.h>

// define xmr settings
#define MONERO_MEMORY 2097152llu
#define MONERO_MASK 0x1FFFF0
#define MONERO_ITER 0x80000

// define aeon settings
#define AEON_MEMORY 1048576llu
#define AEON_MASK 0xFFFF0
#define AEON_ITER 0x40000

namespace c_cryptonight {
	struct cryptonight_ctx {
		uint8_t hash_state[224]; // Need only 200, explicit align
		uint8_t *long_state;
		uint8_t ctx_info[24]; //Use some of the extra memory for flags
	} ;

	struct alloc_msg {
		const char *warning;
	};

	size_t cryptonight_init(size_t use_fast_mem, size_t use_mlock, alloc_msg *msg);

	cryptonight_ctx *cryptonight_alloc_ctx(size_t use_fast_mem, size_t use_mlock, alloc_msg *msg);

	void cryptonight_free_ctx(cryptonight_ctx *ctx);
}

#endif //C_CRYPTONIGHT_CRYPTONIGHT_H
