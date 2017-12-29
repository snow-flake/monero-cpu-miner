//
// Created by Snow Flake on 12/29/17.
//

#include "do_cryptonight.hpp"
#include "cryptonight.hpp"
#include "cryptonight_aesni.hpp"
#include "../logger/logger.hpp"
#include "../hex/hex_conversions.h"


CryptonightWork::CryptonightWork(memory_mode_t memory_mode, hash_mode_t hash_mode, const std::string & input_str):
		memory_mode(memory_mode),
		hash_mode(hash_mode),
		input_size(input_str.size() / 2)
{
	memset(input, 0, sizeof(CryptonightWork::input));
	memset(output, 0, sizeof(CryptonightWork::output));

	//	const std::vector<unsigned char> input_bytes = hex::decodeHex(input_str);
	//	memcpy(input, &input_bytes[0], input_bytes.size());
	hex::hex2bin(input_str, input);
}


const std::string CryptonightWork::input_str() {
	char value[200] = {0};
	hex::bin2hex(input, input_size, value);
	return std::string(value);
}

const std::string CryptonightWork::output_str() {
	char value[200] = {0};
	hex::bin2hex(std::string((char*)output), value);
	return std::string(value);
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
		case currency_monero__aes_disabled__prefetch_disabled: c_cryptonight::currency_monero__aes_disabled__prefetch_disabled(context, work.input, work.input_size, work.output); break;
		case currency_monero__aes_disabled__prefetch_enabled: c_cryptonight::currency_monero__aes_disabled__prefetch_enabled(context, work.input, work.input_size, work.output); break;
		case currency_monero__aes_enabled__prefetch_disabled: c_cryptonight::currency_monero__aes_enabled__prefetch_disabled(context, work.input, work.input_size, work.output); break;
		case currency_monero__aes_enabled__prefetch_enabled: c_cryptonight::currency_monero__aes_enabled__prefetch_enabled(context, work.input, work.input_size, work.output); break;
		case currency_aeon__aes_disabled__prefetch_disabled: c_cryptonight::currency_aeon__aes_disabled__prefetch_disabled(context, work.input, work.input_size, work.output); break;
		case currency_aeon__aes_disabled__prefetch_enabled: c_cryptonight::currency_aeon__aes_disabled__prefetch_enabled(context, work.input, work.input_size, work.output); break;
		case currency_aeon__aes_enabled__prefetch_disabled: c_cryptonight::currency_aeon__aes_enabled__prefetch_disabled(context, work.input, work.input_size, work.output); break;
		case currency_aeon__aes_enabled__prefetch_enabled: c_cryptonight::currency_aeon__aes_enabled__prefetch_enabled(context, work.input, work.input_size, work.output); break;
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

