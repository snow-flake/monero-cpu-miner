//
// Created by Snow Flake on 12/29/17.
//

#include "do_cryptonight.hpp"
#include "cryptonight.hpp"
#include "cryptonight_aesni.hpp"
#include "../logger/logger.hpp"


void _currency_monero__aes_disabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<MONERO_MASK, MONERO_ITER, MONERO_MEMORY, false, false>(work.input, work.input_size, work.output, context);
}

void _currency_monero__aes_disabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<MONERO_MASK, MONERO_ITER, MONERO_MEMORY, false, true>(work.input, work.input_size, work.output, context);
}

void _currency_monero__aes_enabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<MONERO_MASK, MONERO_ITER, MONERO_MEMORY, true, false>(work.input, work.input_size, work.output, context);
}

void _currency_monero__aes_enabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<MONERO_MASK, MONERO_ITER, MONERO_MEMORY, true, true>(work.input, work.input_size, work.output, context);
}

void _currency_aeon__aes_disabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<AEON_MASK, AEON_ITER, AEON_MEMORY, false, false>(work.input, work.input_size, work.output, context);
}

void _currency_aeon__aes_disabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<AEON_MASK, AEON_ITER, AEON_MEMORY, false, true>(work.input, work.input_size, work.output, context);
}

void _currency_aeon__aes_enabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<AEON_MASK, AEON_ITER, AEON_MEMORY, true, false>(work.input, work.input_size, work.output, context);
}

void _currency_aeon__aes_enabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, CryptonightWork & work) {
	c_cryptonight::cryptonight_hash<AEON_MASK, AEON_ITER, AEON_MEMORY, true, true>(work.input, work.input_size, work.output, context);
}


bool do_cryptonight(CryptonightWork & work) {
	c_cryptonight::cryptonight_ctx * context = NULL;
	//	c_cryptonight::cryptonight_init()

	c_cryptonight::alloc_msg msg = { 0 };
	switch(work.memory_mode) {
		case never_use:
			context = c_cryptonight::cryptonight_alloc_ctx(1, 1, &msg);
			break;
		case no_mlck:
			context = c_cryptonight::cryptonight_alloc_ctx(1, 0, &msg);
			break;
		case print_warning:
			context = c_cryptonight::cryptonight_alloc_ctx(1, 1, &msg);
			break;
		case always_use:
			context = c_cryptonight::cryptonight_alloc_ctx(0, 0, NULL);
			break;
	}
	if (context == NULL) {
		logger::log_error(__FILE__, __LINE__, "Memory Alloc Failed");
		if (msg.warning != NULL) {
			logger::log_error(__FILE__, __LINE__, msg.warning);
		}
		return false;
	}

	switch(work.hash_mode) {
		case currency_monero__aes_disabled__prefetch_disabled: _currency_monero__aes_disabled__prefetch_disabled(context, work); break;
		case currency_monero__aes_disabled__prefetch_enabled: _currency_monero__aes_disabled__prefetch_enabled(context, work); break;
		case currency_monero__aes_enabled__prefetch_disabled: _currency_monero__aes_enabled__prefetch_disabled(context, work); break;
		case currency_monero__aes_enabled__prefetch_enabled: _currency_monero__aes_enabled__prefetch_enabled(context, work); break;
		case currency_aeon__aes_disabled__prefetch_disabled: _currency_aeon__aes_disabled__prefetch_disabled(context, work); break;
		case currency_aeon__aes_disabled__prefetch_enabled: _currency_aeon__aes_disabled__prefetch_enabled(context, work); break;
		case currency_aeon__aes_enabled__prefetch_disabled: _currency_aeon__aes_enabled__prefetch_disabled(context, work); break;
		case currency_aeon__aes_enabled__prefetch_enabled: _currency_aeon__aes_enabled__prefetch_enabled(context, work); break;
		default:
			logger::log_error(__FILE__, __LINE__, "Worker selection failed");
			cryptonight_free_ctx(context);
			return false;
	}
	//	if (worker == NULL) {
	//		logger::log_error(__FILE__, __LINE__, "Worker selection failed");
	//		cryptonight_free_ctx(context);
	//		return false;
	//	}
	//
	////	uint64_t* piHashVal = (uint64_t*)(work.output + 24);
	////	uint32_t* piNonce = (uint32_t*)(work.input + 39);
	//
	//	worker(work.input, work.input_size, work.output, context);
	//
	////	size_t nonce_ctr = 0;
	////	constexpr size_t nonce_chunk = 4096; // Needs to be a power of 2
	////	memcpy(result.sJobID, oWork.sJobID, sizeof(job_result::sJobID));
	////
	////	while(globalStates::inst().iGlobalJobNo.load(std::memory_order_relaxed) == iJobNo)
	////	{
	////		*piNonce = ++result.iNonce;
	////
	////		hash_fun(oWork.bWorkBlob, oWork.iWorkSize, result.bResult, ctx);
	////
	////		if (*piHashVal < oWork.iTarget)
	////			executor::inst()->push_event(ex_event(result, oWork.iPoolId));
	////
	////		std::this_thread::yield();
	////	}
	////
	////	consume_work();


	cryptonight_free_ctx(context);
	return true;
}

