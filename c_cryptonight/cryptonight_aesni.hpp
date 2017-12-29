/*
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  */
#pragma once

#include "cryptonight.hpp"
#include <memory.h>
#include <stdio.h>

#ifdef __GNUC__
#include <x86intrin.h>
static inline uint64_t _umul128(uint64_t a, uint64_t b, uint64_t* hi)
{
	unsigned __int128 r = (unsigned __int128)a * (unsigned __int128)b;
	*hi = r >> 64;
	return (uint64_t)r;
}
#define _mm256_set_m128i(v0, v1)  _mm256_insertf128_si256(_mm256_castsi128_si256(v1), (v0), 1)
#else
#include <intrin.h>
#endif // __GNUC__

#if !defined(_LP64) && !defined(_WIN64)
#error You are trying to do a 32-bit build. This will all end in tears. I know it.
#endif

#include "soft_aes.hpp"
#include "cryptonight.hpp"

namespace c_cryptonight {

// This will shift and xor tmp1 into itself as 4 32-bit vals such as
// sl_xor(a1 a2 a3 a4) = a1 (a2^a1) (a3^a2^a1) (a4^a3^a2^a1)
	static inline __m128i sl_xor(__m128i tmp1);

	template<uint8_t rcon>
	static inline void aes_genkey_sub(__m128i *xout0, __m128i *xout2);

	static inline void soft_aes_genkey_sub(__m128i *xout0, __m128i *xout2, uint8_t rcon);

	template<bool SOFT_AES>
	static inline void
	aes_genkey(const __m128i *memory, __m128i *k0, __m128i *k1, __m128i *k2, __m128i *k3, __m128i *k4, __m128i *k5, __m128i *k6, __m128i *k7, __m128i *k8, __m128i *k9);

	static inline void aes_round(__m128i key, __m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3, __m128i *x4, __m128i *x5, __m128i *x6, __m128i *x7);

	static inline void soft_aes_round(__m128i key, __m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3, __m128i *x4, __m128i *x5, __m128i *x6, __m128i *x7);

	template<size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cn_explode_scratchpad(const __m128i *input, __m128i *output);

	template<size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cn_implode_scratchpad(const __m128i *input, __m128i *output);

	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_hash(const void *input, size_t len, void *output, cryptonight_ctx *ctx0);

// This lovely creation will do 2 cn hashes at a time. We have plenty of space on silicon
// to fit temporary vars for two contexts. Function will read len*2 from input and write 64 bytes to output
// We are still limited by L3 cache, so doubling will only work with CPUs where we have more than 2MB to core (Xeons)
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_double_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx);

// This lovelier creation will do 3 cn hashes at a time.
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_triple_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx);

// This even lovelier creation will do 4 cn hashes at a time.
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_quad_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx);

// This most lovely creation will do 5 cn hashes at a time.
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_penta_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx);


	void currency_monero__aes_disabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_monero__aes_disabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_monero__aes_enabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_monero__aes_enabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_aeon__aes_disabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_aeon__aes_disabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_aeon__aes_enabled__prefetch_disabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
	void currency_aeon__aes_enabled__prefetch_enabled(c_cryptonight::cryptonight_ctx * context, uint8_t* input, uint32_t input_size, uint8_t* output);
}
