//
// Created by Snow Flake on 12/28/17.
//

#include "cryptonight_aesni.hpp"
#include "../c_blake/do_blake_hash.hpp"
#include "../c_groestl/do_groestl_hash.hpp"
#include "../c_jh/do_jh_hash.hpp"
#include "../c_skein/do_skein_hash.hpp"
#include "../c_keccak/do_keccak.hpp"

void (* const extra_hashes[4])(const void *, size_t, char *) = {do_blake_hash, do_groestl_hash, do_jh_hash, do_skein_hash};

namespace c_cryptonight {

// This will shift and xor tmp1 into itself as 4 32-bit vals such as
// sl_xor(a1 a2 a3 a4) = a1 (a2^a1) (a3^a2^a1) (a4^a3^a2^a1)
	static inline __m128i sl_xor(__m128i tmp1) {
		__m128i tmp4;
		tmp4 = _mm_slli_si128(tmp1, 0x04);
		tmp1 = _mm_xor_si128(tmp1, tmp4);
		tmp4 = _mm_slli_si128(tmp4, 0x04);
		tmp1 = _mm_xor_si128(tmp1, tmp4);
		tmp4 = _mm_slli_si128(tmp4, 0x04);
		tmp1 = _mm_xor_si128(tmp1, tmp4);
		return tmp1;
	}

	template<uint8_t rcon>
	static inline void aes_genkey_sub(__m128i *xout0, __m128i *xout2) {
		__m128i xout1 = _mm_aeskeygenassist_si128(*xout2, rcon);
		xout1 = _mm_shuffle_epi32(xout1, 0xFF); // see PSHUFD, set all elems to 4th elem
		*xout0 = sl_xor(*xout0);
		*xout0 = _mm_xor_si128(*xout0, xout1);
		xout1 = _mm_aeskeygenassist_si128(*xout0, 0x00);
		xout1 = _mm_shuffle_epi32(xout1, 0xAA); // see PSHUFD, set all elems to 3rd elem
		*xout2 = sl_xor(*xout2);
		*xout2 = _mm_xor_si128(*xout2, xout1);
	}

	static inline void soft_aes_genkey_sub(__m128i *xout0, __m128i *xout2, uint8_t rcon) {
		__m128i xout1 = soft_aeskeygenassist(*xout2, rcon);
		xout1 = _mm_shuffle_epi32(xout1, 0xFF); // see PSHUFD, set all elems to 4th elem
		*xout0 = sl_xor(*xout0);
		*xout0 = _mm_xor_si128(*xout0, xout1);
		xout1 = soft_aeskeygenassist(*xout0, 0x00);
		xout1 = _mm_shuffle_epi32(xout1, 0xAA); // see PSHUFD, set all elems to 3rd elem
		*xout2 = sl_xor(*xout2);
		*xout2 = _mm_xor_si128(*xout2, xout1);
	}


	template<bool SOFT_AES>
	static inline void aes_genkey(const __m128i *memory, __m128i *k0, __m128i *k1, __m128i *k2, __m128i *k3,
								  __m128i *k4, __m128i *k5, __m128i *k6, __m128i *k7, __m128i *k8, __m128i *k9) {
		__m128i xout0, xout2;

		xout0 = _mm_load_si128(memory);
		xout2 = _mm_load_si128(memory + 1);
		*k0 = xout0;
		*k1 = xout2;

		if (SOFT_AES)
			soft_aes_genkey_sub(&xout0, &xout2, 0x01);
		else
			aes_genkey_sub<0x01>(&xout0, &xout2);
		*k2 = xout0;
		*k3 = xout2;

		if (SOFT_AES)
			soft_aes_genkey_sub(&xout0, &xout2, 0x02);
		else
			aes_genkey_sub<0x02>(&xout0, &xout2);
		*k4 = xout0;
		*k5 = xout2;

		if (SOFT_AES)
			soft_aes_genkey_sub(&xout0, &xout2, 0x04);
		else
			aes_genkey_sub<0x04>(&xout0, &xout2);
		*k6 = xout0;
		*k7 = xout2;

		if (SOFT_AES)
			soft_aes_genkey_sub(&xout0, &xout2, 0x08);
		else
			aes_genkey_sub<0x08>(&xout0, &xout2);
		*k8 = xout0;
		*k9 = xout2;
	}

	static inline void aes_round(__m128i key, __m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3, __m128i *x4, __m128i *x5, __m128i *x6, __m128i *x7) {
		*x0 = _mm_aesenc_si128(*x0, key);
		*x1 = _mm_aesenc_si128(*x1, key);
		*x2 = _mm_aesenc_si128(*x2, key);
		*x3 = _mm_aesenc_si128(*x3, key);
		*x4 = _mm_aesenc_si128(*x4, key);
		*x5 = _mm_aesenc_si128(*x5, key);
		*x6 = _mm_aesenc_si128(*x6, key);
		*x7 = _mm_aesenc_si128(*x7, key);
	}

	static inline void soft_aes_round(__m128i key, __m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3, __m128i *x4, __m128i *x5, __m128i *x6, __m128i *x7) {
		*x0 = soft_aesenc(*x0, key);
		*x1 = soft_aesenc(*x1, key);
		*x2 = soft_aesenc(*x2, key);
		*x3 = soft_aesenc(*x3, key);
		*x4 = soft_aesenc(*x4, key);
		*x5 = soft_aesenc(*x5, key);
		*x6 = soft_aesenc(*x6, key);
		*x7 = soft_aesenc(*x7, key);
	}

	template<size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cn_explode_scratchpad(const __m128i *input, __m128i *output) {
		// This is more than we have registers, compiler will assign 2 keys on the stack
		__m128i xin0, xin1, xin2, xin3, xin4, xin5, xin6, xin7;
		__m128i k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;

		aes_genkey<SOFT_AES>(input, &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9);

		xin0 = _mm_load_si128(input + 4);
		xin1 = _mm_load_si128(input + 5);
		xin2 = _mm_load_si128(input + 6);
		xin3 = _mm_load_si128(input + 7);
		xin4 = _mm_load_si128(input + 8);
		xin5 = _mm_load_si128(input + 9);
		xin6 = _mm_load_si128(input + 10);
		xin7 = _mm_load_si128(input + 11);

		for (size_t i = 0; i < MEM / sizeof(__m128i); i += 8) {
			if (SOFT_AES) {
				soft_aes_round(k0, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k1, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k2, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k3, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k4, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k5, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k6, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k7, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k8, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				soft_aes_round(k9, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
			} else {
				aes_round(k0, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k1, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k2, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k3, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k4, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k5, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k6, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k7, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k8, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
				aes_round(k9, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
			}

			_mm_store_si128(output + i + 0, xin0);
			_mm_store_si128(output + i + 1, xin1);
			_mm_store_si128(output + i + 2, xin2);
			_mm_store_si128(output + i + 3, xin3);

			if (PREFETCH)
				_mm_prefetch((const char *) output + i + 0, _MM_HINT_T2);

			_mm_store_si128(output + i + 4, xin4);
			_mm_store_si128(output + i + 5, xin5);
			_mm_store_si128(output + i + 6, xin6);
			_mm_store_si128(output + i + 7, xin7);

			if (PREFETCH)
				_mm_prefetch((const char *) output + i + 4, _MM_HINT_T2);
		}
	}

	template<size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cn_implode_scratchpad(const __m128i *input, __m128i *output) {
		// This is more than we have registers, compiler will assign 2 keys on the stack
		__m128i xout0, xout1, xout2, xout3, xout4, xout5, xout6, xout7;
		__m128i k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;

		aes_genkey<SOFT_AES>(output + 2, &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9);

		xout0 = _mm_load_si128(output + 4);
		xout1 = _mm_load_si128(output + 5);
		xout2 = _mm_load_si128(output + 6);
		xout3 = _mm_load_si128(output + 7);
		xout4 = _mm_load_si128(output + 8);
		xout5 = _mm_load_si128(output + 9);
		xout6 = _mm_load_si128(output + 10);
		xout7 = _mm_load_si128(output + 11);

		for (size_t i = 0; i < MEM / sizeof(__m128i); i += 8) {
			if (PREFETCH)
				_mm_prefetch((const char *) input + i + 0, _MM_HINT_NTA);

			xout0 = _mm_xor_si128(_mm_load_si128(input + i + 0), xout0);
			xout1 = _mm_xor_si128(_mm_load_si128(input + i + 1), xout1);
			xout2 = _mm_xor_si128(_mm_load_si128(input + i + 2), xout2);
			xout3 = _mm_xor_si128(_mm_load_si128(input + i + 3), xout3);

			if (PREFETCH)
				_mm_prefetch((const char *) input + i + 4, _MM_HINT_NTA);

			xout4 = _mm_xor_si128(_mm_load_si128(input + i + 4), xout4);
			xout5 = _mm_xor_si128(_mm_load_si128(input + i + 5), xout5);
			xout6 = _mm_xor_si128(_mm_load_si128(input + i + 6), xout6);
			xout7 = _mm_xor_si128(_mm_load_si128(input + i + 7), xout7);

			if (SOFT_AES) {
				soft_aes_round(k0, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k1, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k2, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k3, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k4, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k5, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k6, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k7, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k8, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				soft_aes_round(k9, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
			} else {
				aes_round(k0, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k1, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k2, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k3, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k4, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k5, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k6, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k7, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k8, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
				aes_round(k9, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
			}
		}

		_mm_store_si128(output + 4, xout0);
		_mm_store_si128(output + 5, xout1);
		_mm_store_si128(output + 6, xout2);
		_mm_store_si128(output + 7, xout3);
		_mm_store_si128(output + 8, xout4);
		_mm_store_si128(output + 9, xout5);
		_mm_store_si128(output + 10, xout6);
		_mm_store_si128(output + 11, xout7);
	}

	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_hash(const void *input, size_t len, void *output, cryptonight_ctx *ctx0) {
		do_keccak((const uint8_t *) input, len, ctx0->hash_state, 200);

		// Optim - 99% time boundary
		cn_explode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx0->hash_state, (__m128i *) ctx0->long_state);

		uint8_t *l0 = ctx0->long_state;
		uint64_t *h0 = (uint64_t *) ctx0->hash_state;

		uint64_t al0 = h0[0] ^h0[4];
		uint64_t ah0 = h0[1] ^h0[5];
		__m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);

		uint64_t idx0 = h0[0] ^h0[4];

		// Optim - 90% time boundary
		for (size_t i = 0; i < ITERATIONS; i++) {
			__m128i cx;
			cx = _mm_load_si128((__m128i *) &l0[idx0 & MASK]);

			if (SOFT_AES)
				cx = soft_aesenc(cx, _mm_set_epi64x(ah0, al0));
			else
				cx = _mm_aesenc_si128(cx, _mm_set_epi64x(ah0, al0));

			_mm_store_si128((__m128i *) &l0[idx0 & MASK], _mm_xor_si128(bx0, cx));
			idx0 = _mm_cvtsi128_si64(cx);
			bx0 = cx;

			if (PREFETCH)
				_mm_prefetch((const char *) &l0[idx0 & MASK], _MM_HINT_T0);

			uint64_t hi, lo, cl, ch;
			cl = ((uint64_t *) &l0[idx0 & MASK])[0];
			ch = ((uint64_t *) &l0[idx0 & MASK])[1];

			lo = _umul128(idx0, cl, &hi);

			al0 += hi;
			ah0 += lo;
			((uint64_t *) &l0[idx0 & MASK])[0] = al0;
			((uint64_t *) &l0[idx0 & MASK])[1] = ah0;
			ah0 ^= ch;
			al0 ^= cl;
			idx0 = al0;

			if (PREFETCH)
				_mm_prefetch((const char *) &l0[idx0 & MASK], _MM_HINT_T0);
		}

		// Optim - 90% time boundary
		cn_implode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx0->long_state, (__m128i *) ctx0->hash_state);

		// Optim - 99% time boundary

		do_keccakf((uint64_t *) ctx0->hash_state, 24);
		extra_hashes[ctx0->hash_state[0] & 3](ctx0->hash_state, 200, (char *) output);
	}

// This lovely creation will do 2 cn hashes at a time. We have plenty of space on silicon
// to fit temporary vars for two contexts. Function will read len*2 from input and write 64 bytes to output
// We are still limited by L3 cache, so doubling will only work with CPUs where we have more than 2MB to core (Xeons)
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_double_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx) {
		do_keccak((const uint8_t *) input, len, ctx[0]->hash_state, 200);
		do_keccak((const uint8_t *) input + len, len, ctx[1]->hash_state, 200);

		// Optim - 99% time boundary
		cn_explode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[0]->hash_state, (__m128i *) ctx[0]->long_state);
		cn_explode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[1]->hash_state, (__m128i *) ctx[1]->long_state);

		uint8_t *l0 = ctx[0]->long_state;
		uint64_t *h0 = (uint64_t *) ctx[0]->hash_state;
		uint8_t *l1 = ctx[1]->long_state;
		uint64_t *h1 = (uint64_t *) ctx[1]->hash_state;

		uint64_t axl0 = h0[0] ^h0[4];
		uint64_t axh0 = h0[1] ^h0[5];
		__m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);
		uint64_t axl1 = h1[0] ^h1[4];
		uint64_t axh1 = h1[1] ^h1[5];
		__m128i bx1 = _mm_set_epi64x(h1[3] ^ h1[7], h1[2] ^ h1[6]);

		uint64_t idx0 = h0[0] ^h0[4];
		uint64_t idx1 = h1[0] ^h1[4];

		// Optim - 90% time boundary
		for (size_t i = 0; i < ITERATIONS; i++) {
			__m128i cx;
			cx = _mm_load_si128((__m128i *) &l0[idx0 & MASK]);

			if (SOFT_AES)
				cx = soft_aesenc(cx, _mm_set_epi64x(axh0, axl0));
			else
				cx = _mm_aesenc_si128(cx, _mm_set_epi64x(axh0, axl0));

			_mm_store_si128((__m128i *) &l0[idx0 & MASK], _mm_xor_si128(bx0, cx));
			idx0 = _mm_cvtsi128_si64(cx);
			bx0 = cx;

			if (PREFETCH)
				_mm_prefetch((const char *) &l0[idx0 & MASK], _MM_HINT_T0);

			cx = _mm_load_si128((__m128i *) &l1[idx1 & MASK]);

			if (SOFT_AES)
				cx = soft_aesenc(cx, _mm_set_epi64x(axh1, axl1));
			else
				cx = _mm_aesenc_si128(cx, _mm_set_epi64x(axh1, axl1));

			_mm_store_si128((__m128i *) &l1[idx1 & MASK], _mm_xor_si128(bx1, cx));
			idx1 = _mm_cvtsi128_si64(cx);
			bx1 = cx;

			if (PREFETCH)
				_mm_prefetch((const char *) &l1[idx1 & MASK], _MM_HINT_T0);

			uint64_t hi, lo, cl, ch;
			cl = ((uint64_t *) &l0[idx0 & MASK])[0];
			ch = ((uint64_t *) &l0[idx0 & MASK])[1];

			lo = _umul128(idx0, cl, &hi);

			axl0 += hi;
			axh0 += lo;
			((uint64_t *) &l0[idx0 & MASK])[0] = axl0;
			((uint64_t *) &l0[idx0 & MASK])[1] = axh0;
			axh0 ^= ch;
			axl0 ^= cl;
			idx0 = axl0;

			if (PREFETCH)
				_mm_prefetch((const char *) &l0[idx0 & MASK], _MM_HINT_T0);

			cl = ((uint64_t *) &l1[idx1 & MASK])[0];
			ch = ((uint64_t *) &l1[idx1 & MASK])[1];

			lo = _umul128(idx1, cl, &hi);

			axl1 += hi;
			axh1 += lo;
			((uint64_t *) &l1[idx1 & MASK])[0] = axl1;
			((uint64_t *) &l1[idx1 & MASK])[1] = axh1;
			axh1 ^= ch;
			axl1 ^= cl;
			idx1 = axl1;

			if (PREFETCH)
				_mm_prefetch((const char *) &l1[idx1 & MASK], _MM_HINT_T0);
		}

		// Optim - 90% time boundary
		cn_implode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[0]->long_state, (__m128i *) ctx[0]->hash_state);
		cn_implode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[1]->long_state, (__m128i *) ctx[1]->hash_state);

		// Optim - 99% time boundary

		do_keccakf((uint64_t *) ctx[0]->hash_state, 24);
		extra_hashes[ctx[0]->hash_state[0] & 3](ctx[0]->hash_state, 200, (char *) output);
		do_keccakf((uint64_t *) ctx[1]->hash_state, 24);
		extra_hashes[ctx[1]->hash_state[0] & 3](ctx[1]->hash_state, 200, (char *) output + 32);
	}

#define CN_STEP1(a, b, c, l, ptr, idx)                \
    a = _mm_xor_si128(a, c);                \
    idx = _mm_cvtsi128_si64(a);                \
    ptr = (__m128i *)&l[idx & MASK];            \
    if(PREFETCH)                        \
        _mm_prefetch((const char*)ptr, _MM_HINT_T0);    \
    c = _mm_load_si128(ptr)

#define CN_STEP2(a, b, c, l, ptr, idx)                \
    if(SOFT_AES)                        \
        c = soft_aesenc(c, a);                \
    else                            \
        c = _mm_aesenc_si128(c, a);            \
    b = _mm_xor_si128(b, c);                \
    _mm_store_si128(ptr, b)

#define CN_STEP3(a, b, c, l, ptr, idx)                \
    idx = _mm_cvtsi128_si64(c);                \
    ptr = (__m128i *)&l[idx & MASK];            \
    if(PREFETCH)                        \
        _mm_prefetch((const char*)ptr, _MM_HINT_T0);    \
    b = _mm_load_si128(ptr)

#define CN_STEP4(a, b, c, l, ptr, idx)                \
    lo = _umul128(idx, _mm_cvtsi128_si64(b), &hi);        \
    a = _mm_add_epi64(a, _mm_set_epi64x(lo, hi));        \
    _mm_store_si128(ptr, a)

// This lovelier creation will do 3 cn hashes at a time.
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_triple_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx) {
		for (size_t i = 0; i < 3; i++) {
			do_keccak((const uint8_t *) input + len * i, len, ctx[i]->hash_state, 200);
			cn_explode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[i]->hash_state, (__m128i *) ctx[i]->long_state);
		}

		uint8_t *l0 = ctx[0]->long_state;
		uint64_t *h0 = (uint64_t *) ctx[0]->hash_state;
		uint8_t *l1 = ctx[1]->long_state;
		uint64_t *h1 = (uint64_t *) ctx[1]->hash_state;
		uint8_t *l2 = ctx[2]->long_state;
		uint64_t *h2 = (uint64_t *) ctx[2]->hash_state;

		__m128i ax0 = _mm_set_epi64x(h0[1] ^ h0[5], h0[0] ^ h0[4]);
		__m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);
		__m128i ax1 = _mm_set_epi64x(h1[1] ^ h1[5], h1[0] ^ h1[4]);
		__m128i bx1 = _mm_set_epi64x(h1[3] ^ h1[7], h1[2] ^ h1[6]);
		__m128i ax2 = _mm_set_epi64x(h2[1] ^ h2[5], h2[0] ^ h2[4]);
		__m128i bx2 = _mm_set_epi64x(h2[3] ^ h2[7], h2[2] ^ h2[6]);
		__m128i cx0 = _mm_set_epi64x(0, 0);
		__m128i cx1 = _mm_set_epi64x(0, 0);
		__m128i cx2 = _mm_set_epi64x(0, 0);

		for (size_t i = 0; i < ITERATIONS / 2; i++) {
			uint64_t idx0, idx1, idx2, hi, lo;
			__m128i *ptr0, *ptr1, *ptr2;

			// EVEN ROUND
			CN_STEP1(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP1(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP1(ax2, bx2, cx2, l2, ptr2, idx2);

			CN_STEP2(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP2(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP2(ax2, bx2, cx2, l2, ptr2, idx2);

			CN_STEP3(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP3(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP3(ax2, bx2, cx2, l2, ptr2, idx2);

			CN_STEP4(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP4(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP4(ax2, bx2, cx2, l2, ptr2, idx2);

			// ODD ROUND
			CN_STEP1(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP1(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP1(ax2, cx2, bx2, l2, ptr2, idx2);

			CN_STEP2(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP2(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP2(ax2, cx2, bx2, l2, ptr2, idx2);

			CN_STEP3(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP3(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP3(ax2, cx2, bx2, l2, ptr2, idx2);

			CN_STEP4(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP4(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP4(ax2, cx2, bx2, l2, ptr2, idx2);
		}

		for (size_t i = 0; i < 3; i++) {
			cn_implode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[i]->long_state, (__m128i *) ctx[i]->hash_state);
			do_keccakf((uint64_t *) ctx[i]->hash_state, 24);
			extra_hashes[ctx[i]->hash_state[0] & 3](ctx[i]->hash_state, 200, (char *) output + 32 * i);
		}
	}

// This even lovelier creation will do 4 cn hashes at a time.
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_quad_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx) {
		for (size_t i = 0; i < 4; i++) {
			do_keccak((const uint8_t *) input + len * i, len, ctx[i]->hash_state, 200);
			cn_explode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[i]->hash_state, (__m128i *) ctx[i]->long_state);
		}

		uint8_t *l0 = ctx[0]->long_state;
		uint64_t *h0 = (uint64_t *) ctx[0]->hash_state;
		uint8_t *l1 = ctx[1]->long_state;
		uint64_t *h1 = (uint64_t *) ctx[1]->hash_state;
		uint8_t *l2 = ctx[2]->long_state;
		uint64_t *h2 = (uint64_t *) ctx[2]->hash_state;
		uint8_t *l3 = ctx[3]->long_state;
		uint64_t *h3 = (uint64_t *) ctx[3]->hash_state;

		__m128i ax0 = _mm_set_epi64x(h0[1] ^ h0[5], h0[0] ^ h0[4]);
		__m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);
		__m128i ax1 = _mm_set_epi64x(h1[1] ^ h1[5], h1[0] ^ h1[4]);
		__m128i bx1 = _mm_set_epi64x(h1[3] ^ h1[7], h1[2] ^ h1[6]);
		__m128i ax2 = _mm_set_epi64x(h2[1] ^ h2[5], h2[0] ^ h2[4]);
		__m128i bx2 = _mm_set_epi64x(h2[3] ^ h2[7], h2[2] ^ h2[6]);
		__m128i ax3 = _mm_set_epi64x(h3[1] ^ h3[5], h3[0] ^ h3[4]);
		__m128i bx3 = _mm_set_epi64x(h3[3] ^ h3[7], h3[2] ^ h3[6]);
		__m128i cx0 = _mm_set_epi64x(0, 0);
		__m128i cx1 = _mm_set_epi64x(0, 0);
		__m128i cx2 = _mm_set_epi64x(0, 0);
		__m128i cx3 = _mm_set_epi64x(0, 0);

		for (size_t i = 0; i < ITERATIONS / 2; i++) {
			uint64_t idx0, idx1, idx2, idx3, hi, lo;
			__m128i *ptr0, *ptr1, *ptr2, *ptr3;

			// EVEN ROUND
			CN_STEP1(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP1(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP1(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP1(ax3, bx3, cx3, l3, ptr3, idx3);

			CN_STEP2(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP2(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP2(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP2(ax3, bx3, cx3, l3, ptr3, idx3);

			CN_STEP3(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP3(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP3(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP3(ax3, bx3, cx3, l3, ptr3, idx3);

			CN_STEP4(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP4(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP4(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP4(ax3, bx3, cx3, l3, ptr3, idx3);

			// ODD ROUND
			CN_STEP1(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP1(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP1(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP1(ax3, cx3, bx3, l3, ptr3, idx3);

			CN_STEP2(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP2(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP2(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP2(ax3, cx3, bx3, l3, ptr3, idx3);

			CN_STEP3(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP3(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP3(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP3(ax3, cx3, bx3, l3, ptr3, idx3);

			CN_STEP4(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP4(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP4(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP4(ax3, cx3, bx3, l3, ptr3, idx3);
		}

		for (size_t i = 0; i < 4; i++) {
			cn_implode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[i]->long_state, (__m128i *) ctx[i]->hash_state);
			do_keccakf((uint64_t *) ctx[i]->hash_state, 24);
			extra_hashes[ctx[i]->hash_state[0] & 3](ctx[i]->hash_state, 200, (char *) output + 32 * i);
		}
	}

// This most lovely creation will do 5 cn hashes at a time.
	template<size_t MASK, size_t ITERATIONS, size_t MEM, bool SOFT_AES, bool PREFETCH>
	void cryptonight_penta_hash(const void *input, size_t len, void *output, cryptonight_ctx **ctx) {
		for (size_t i = 0; i < 5; i++) {
			do_keccak((const uint8_t *) input + len * i, len, ctx[i]->hash_state, 200);
			cn_explode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[i]->hash_state, (__m128i *) ctx[i]->long_state);
		}

		uint8_t *l0 = ctx[0]->long_state;
		uint64_t *h0 = (uint64_t *) ctx[0]->hash_state;
		uint8_t *l1 = ctx[1]->long_state;
		uint64_t *h1 = (uint64_t *) ctx[1]->hash_state;
		uint8_t *l2 = ctx[2]->long_state;
		uint64_t *h2 = (uint64_t *) ctx[2]->hash_state;
		uint8_t *l3 = ctx[3]->long_state;
		uint64_t *h3 = (uint64_t *) ctx[3]->hash_state;
		uint8_t *l4 = ctx[4]->long_state;
		uint64_t *h4 = (uint64_t *) ctx[4]->hash_state;

		__m128i ax0 = _mm_set_epi64x(h0[1] ^ h0[5], h0[0] ^ h0[4]);
		__m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);
		__m128i ax1 = _mm_set_epi64x(h1[1] ^ h1[5], h1[0] ^ h1[4]);
		__m128i bx1 = _mm_set_epi64x(h1[3] ^ h1[7], h1[2] ^ h1[6]);
		__m128i ax2 = _mm_set_epi64x(h2[1] ^ h2[5], h2[0] ^ h2[4]);
		__m128i bx2 = _mm_set_epi64x(h2[3] ^ h2[7], h2[2] ^ h2[6]);
		__m128i ax3 = _mm_set_epi64x(h3[1] ^ h3[5], h3[0] ^ h3[4]);
		__m128i bx3 = _mm_set_epi64x(h3[3] ^ h3[7], h3[2] ^ h3[6]);
		__m128i ax4 = _mm_set_epi64x(h4[1] ^ h4[5], h4[0] ^ h4[4]);
		__m128i bx4 = _mm_set_epi64x(h4[3] ^ h4[7], h4[2] ^ h4[6]);
		__m128i cx0 = _mm_set_epi64x(0, 0);
		__m128i cx1 = _mm_set_epi64x(0, 0);
		__m128i cx2 = _mm_set_epi64x(0, 0);
		__m128i cx3 = _mm_set_epi64x(0, 0);
		__m128i cx4 = _mm_set_epi64x(0, 0);

		for (size_t i = 0; i < ITERATIONS / 2; i++) {
			uint64_t idx0, idx1, idx2, idx3, idx4, hi, lo;
			__m128i *ptr0, *ptr1, *ptr2, *ptr3, *ptr4;

			// EVEN ROUND
			CN_STEP1(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP1(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP1(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP1(ax3, bx3, cx3, l3, ptr3, idx3);
			CN_STEP1(ax4, bx4, cx4, l4, ptr4, idx4);

			CN_STEP2(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP2(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP2(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP2(ax3, bx3, cx3, l3, ptr3, idx3);
			CN_STEP2(ax4, bx4, cx4, l4, ptr4, idx4);

			CN_STEP3(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP3(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP3(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP3(ax3, bx3, cx3, l3, ptr3, idx3);
			CN_STEP3(ax4, bx4, cx4, l4, ptr4, idx4);

			CN_STEP4(ax0, bx0, cx0, l0, ptr0, idx0);
			CN_STEP4(ax1, bx1, cx1, l1, ptr1, idx1);
			CN_STEP4(ax2, bx2, cx2, l2, ptr2, idx2);
			CN_STEP4(ax3, bx3, cx3, l3, ptr3, idx3);
			CN_STEP4(ax4, bx4, cx4, l4, ptr4, idx4);

			// ODD ROUND
			CN_STEP1(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP1(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP1(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP1(ax3, cx3, bx3, l3, ptr3, idx3);
			CN_STEP1(ax4, cx4, bx4, l4, ptr4, idx4);

			CN_STEP2(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP2(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP2(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP2(ax3, cx3, bx3, l3, ptr3, idx3);
			CN_STEP2(ax4, cx4, bx4, l4, ptr4, idx4);

			CN_STEP3(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP3(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP3(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP3(ax3, cx3, bx3, l3, ptr3, idx3);
			CN_STEP3(ax4, cx4, bx4, l4, ptr4, idx4);

			CN_STEP4(ax0, cx0, bx0, l0, ptr0, idx0);
			CN_STEP4(ax1, cx1, bx1, l1, ptr1, idx1);
			CN_STEP4(ax2, cx2, bx2, l2, ptr2, idx2);
			CN_STEP4(ax3, cx3, bx3, l3, ptr3, idx3);
			CN_STEP4(ax4, cx4, bx4, l4, ptr4, idx4);
		}

		for (size_t i = 0; i < 5; i++) {
			cn_implode_scratchpad<MEM, SOFT_AES, PREFETCH>((__m128i *) ctx[i]->long_state, (__m128i *) ctx[i]->hash_state);
			do_keccakf((uint64_t *) ctx[i]->hash_state, 24);
			extra_hashes[ctx[i]->hash_state[0] & 3](ctx[i]->hash_state, 200, (char *) output + 32 * i);
		}
	}
}