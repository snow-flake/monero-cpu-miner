//
// Created by Snow Flake on 12/28/17.
//

#ifdef __GNUC__
#include <mm_malloc.h>
#else
#include <malloc.h>
#endif // __GNUC__

#if defined(__APPLE__)
#include <mach/vm_statistics.h>
#endif

#include <sys/mman.h>
#include "cryptonight.hpp"
#include "cryptonight_aesni.hpp"


namespace c_cryptonight {

	size_t cryptonight_init(size_t use_fast_mem, size_t use_mlock, alloc_msg *msg) {
		return 1;
	}

	cryptonight_ctx *cryptonight_alloc_ctx(size_t use_fast_mem, size_t use_mlock, alloc_msg *msg) {
		size_t hashMemSize;
		if (true) {
			// ::jconf::inst()->IsCurrencyMonero()
			hashMemSize = MONERO_MEMORY;
		} else {
			hashMemSize = AEON_MEMORY;
		}
		cryptonight_ctx *ptr = (cryptonight_ctx *) _mm_malloc(sizeof(cryptonight_ctx), 4096);

		if (use_fast_mem == 0) {
			// use 2MiB aligned memory
			ptr->long_state = (uint8_t *) _mm_malloc(hashMemSize, hashMemSize);
			ptr->ctx_info[0] = 0;
			ptr->ctx_info[1] = 0;
			return ptr;
		}

#if defined(__APPLE__)
		ptr->long_state = (uint8_t *) mmap(0, hashMemSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
#elif defined(__FreeBSD__)
		ptr->long_state = (uint8_t*)mmap(0, hashMemSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_ALIGNED_SUPER | MAP_PREFAULT_READ, -1, 0);
#else
		ptr->long_state = (uint8_t*)mmap(0, hashMemSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_POPULATE, 0, 0);
#endif

		if (ptr->long_state == MAP_FAILED) {
			_mm_free(ptr);
			msg->warning = "mmap failed";
			return NULL;
		}

		ptr->ctx_info[0] = 1;

		if (madvise(ptr->long_state, hashMemSize, MADV_RANDOM | MADV_WILLNEED) != 0) {
			msg->warning = "madvise failed";
		}

		ptr->ctx_info[1] = 0;
		if (use_mlock != 0 && mlock(ptr->long_state, hashMemSize) != 0){
			msg->warning = "mlock failed";
		}
		else {
			ptr->ctx_info[1] = 1;
		}

		return ptr;
	}

	void cryptonight_free_ctx(cryptonight_ctx *ctx) {
		size_t hashMemSize;
		if (true) {
			// ::jconf::inst()->IsCurrencyMonero()
			hashMemSize = MONERO_MEMORY;
		} else {
			hashMemSize = AEON_MEMORY;
		}
		if (ctx->ctx_info[0] != 0) {
			if (ctx->ctx_info[1] != 0){
				munlock(ctx->long_state, hashMemSize);
			}
			munmap(ctx->long_state, hashMemSize);
		} else {
			_mm_free(ctx->long_state);
		}

		_mm_free(ctx);
	}

}