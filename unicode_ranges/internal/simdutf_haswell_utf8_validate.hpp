// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright (c) 2026 unicode_ranges contributors
//
// Provenance:
// - Adapted from: simdutf
// - Original files:
//   - include/simdutf/portability.h
//   - src/simdutf/haswell/begin.h
//   - src/simdutf/haswell/simd.h
//   - src/generic/buf_block_reader.h
//   - src/generic/utf8_validation/utf8_lookup4_algorithm.h
//   - src/generic/utf8_validation/utf8_validator.h
// - Upstream version: v7.7.0
// - Original license: MIT OR Apache-2.0
// - Changes for unicode_ranges:
//   - reduced to runtime AVX2 UTF-8 validation success/fail only
//   - removed simdutf result/error types and non-UTF-8 features
//   - mapped runtime dispatch into a small local helper API

#ifndef UTF8_RANGES_INTERNAL_SIMDUTF_HASWELL_UTF8_VALIDATE_HPP
#define UTF8_RANGES_INTERNAL_SIMDUTF_HASWELL_UTF8_VALIDATE_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

#if defined(__x86_64__) || defined(_M_AMD64)
	#if defined(_MSC_VER)
		#include <intrin.h>
	#endif
	#include <immintrin.h>
#endif

#if defined(_MSC_VER)
	#define UTF8_RANGES_SIMDUTF_REALLY_INLINE __forceinline
	#define UTF8_RANGES_SIMDUTF_LIKELY(x) (x)
#else
	#define UTF8_RANGES_SIMDUTF_REALLY_INLINE inline __attribute__((always_inline))
	#define UTF8_RANGES_SIMDUTF_LIKELY(x) __builtin_expect(!!(x), 1)
#endif

#if (defined(__x86_64__) || defined(_M_AMD64)) && defined(__clang__)
	#define UTF8_RANGES_SIMDUTF_TARGET_REGION                                     \
		_Pragma("clang attribute push(__attribute__((target(\"avx2,popcnt\"))), apply_to=function)")
	#define UTF8_RANGES_SIMDUTF_UNTARGET_REGION _Pragma("clang attribute pop")
#elif (defined(__x86_64__) || defined(_M_AMD64)) && defined(__GNUC__)
	#define UTF8_RANGES_SIMDUTF_TARGET_REGION                                     \
		_Pragma("GCC push_options") _Pragma("GCC target(\"avx2,popcnt\")")
	#define UTF8_RANGES_SIMDUTF_UNTARGET_REGION _Pragma("GCC pop_options")
#else
	#define UTF8_RANGES_SIMDUTF_TARGET_REGION
	#define UTF8_RANGES_SIMDUTF_UNTARGET_REGION
#endif

namespace unicode_ranges::details::internal
{

inline bool simdutf_haswell_runtime_available() noexcept
{
#if defined(__x86_64__) || defined(_M_AMD64)
	#if defined(_MSC_VER)
		int cpu_info[4]{};
		__cpuidex(cpu_info, 1, 0);
		const std::uint32_t ecx = static_cast<std::uint32_t>(cpu_info[2]);
		constexpr std::uint32_t osxsave_mask = (1u << 27) | (1u << 26);
		if ((ecx & osxsave_mask) != osxsave_mask)
		{
			return false;
		}

		const std::uint64_t xcr0 = static_cast<std::uint64_t>(_xgetbv(0));
		if ((xcr0 & (1ull << 2)) == 0)
		{
			return false;
		}

		__cpuidex(cpu_info, 7, 0);
		const std::uint32_t ebx = static_cast<std::uint32_t>(cpu_info[1]);
		return (ebx & (1u << 5)) != 0;
	#elif defined(__GNUC__) || defined(__clang__)
		return __builtin_cpu_supports("avx2");
	#else
		return false;
	#endif
#else
	return false;
#endif
}

} // namespace unicode_ranges::details::internal

UTF8_RANGES_SIMDUTF_TARGET_REGION

namespace unicode_ranges::details::internal::simdutf_haswell_validation
{

#if defined(__x86_64__) || defined(_M_AMD64)

namespace simd
{

template <typename Child>
struct base
{
	__m256i value;

	UTF8_RANGES_SIMDUTF_REALLY_INLINE base() : value{ __m256i{} } {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE base(const __m256i _value) : value(_value) {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE operator const __m256i& () const { return this->value; }

	UTF8_RANGES_SIMDUTF_REALLY_INLINE Child operator |(const Child other) const
	{
		return _mm256_or_si256(*this, other);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE Child operator &(const Child other) const
	{
		return _mm256_and_si256(*this, other);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE Child operator ^(const Child other) const
	{
		return _mm256_xor_si256(*this, other);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE Child& operator |=(const Child other)
	{
		auto this_cast = static_cast<Child*>(this);
		*this_cast = *this_cast | other;
		return *this_cast;
	}
};

template <typename T>
struct simd8;

template <typename T, typename Mask = simd8<bool>>
struct base8 : base<simd8<T>>
{
	UTF8_RANGES_SIMDUTF_REALLY_INLINE base8() : base<simd8<T>>() {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE base8(const __m256i _value) : base<simd8<T>>(_value) {}

	friend UTF8_RANGES_SIMDUTF_REALLY_INLINE Mask operator ==(const simd8<T> lhs, const simd8<T> rhs)
	{
		return _mm256_cmpeq_epi8(lhs, rhs);
	}

	template <int N = 1>
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<T> prev(const simd8<T> prev_chunk) const
	{
		return _mm256_alignr_epi8(*this, _mm256_permute2x128_si256(prev_chunk, *this, 0x21), 16 - N);
	}
};

template <>
struct simd8<bool> : base8<bool>
{
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8() : base8<bool>(_mm256_setzero_si256()) {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8(const __m256i _value) : base8<bool>(_value) {}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE std::uint32_t to_bitmask() const
	{
		return static_cast<std::uint32_t>(_mm256_movemask_epi8(this->value));
	}
};

template <typename T>
struct base8_numeric : base8<T>
{
	static UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<T> splat(T _value)
	{
		return _mm256_set1_epi8(_value);
	}

	static UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<T> load(const T values[32])
	{
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
	}

	static UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<T> repeat_16(
		T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7,
		T v8, T v9, T v10, T v11, T v12, T v13, T v14, T v15)
	{
		return simd8<T>(
			v0, v1, v2, v3, v4, v5, v6, v7,
			v8, v9, v10, v11, v12, v13, v14, v15,
			v0, v1, v2, v3, v4, v5, v6, v7,
			v8, v9, v10, v11, v12, v13, v14, v15);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE base8_numeric() : base8<T>() {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE base8_numeric(const __m256i _value) : base8<T>(_value) {}

	template <typename L>
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<L> lookup_16(simd8<L> lookup_table) const
	{
		return _mm256_shuffle_epi8(lookup_table, *this);
	}

	template <typename L>
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<L> lookup_16(
		L replace0, L replace1, L replace2, L replace3,
		L replace4, L replace5, L replace6, L replace7,
		L replace8, L replace9, L replace10, L replace11,
		L replace12, L replace13, L replace14, L replace15) const
	{
		return lookup_16(simd8<L>::repeat_16(
			replace0, replace1, replace2, replace3,
			replace4, replace5, replace6, replace7,
			replace8, replace9, replace10, replace11,
			replace12, replace13, replace14, replace15));
	}
};

template <>
struct simd8<std::uint8_t> : base8_numeric<std::uint8_t>
{
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8() : base8_numeric<std::uint8_t>() {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8(const __m256i _value) : base8_numeric<std::uint8_t>(_value) {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8(std::uint8_t _value) : simd8(splat(_value)) {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8(const std::uint8_t values[32]) : simd8(load(values)) {}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE
	simd8(
		std::uint8_t v0, std::uint8_t v1, std::uint8_t v2, std::uint8_t v3,
		std::uint8_t v4, std::uint8_t v5, std::uint8_t v6, std::uint8_t v7,
		std::uint8_t v8, std::uint8_t v9, std::uint8_t v10, std::uint8_t v11,
		std::uint8_t v12, std::uint8_t v13, std::uint8_t v14, std::uint8_t v15,
		std::uint8_t v16, std::uint8_t v17, std::uint8_t v18, std::uint8_t v19,
		std::uint8_t v20, std::uint8_t v21, std::uint8_t v22, std::uint8_t v23,
		std::uint8_t v24, std::uint8_t v25, std::uint8_t v26, std::uint8_t v27,
		std::uint8_t v28, std::uint8_t v29, std::uint8_t v30, std::uint8_t v31)
		: simd8(_mm256_setr_epi8(
			v0, v1, v2, v3, v4, v5, v6, v7,
			v8, v9, v10, v11, v12, v13, v14, v15,
			v16, v17, v18, v19, v20, v21, v22, v23,
			v24, v25, v26, v27, v28, v29, v30, v31))
	{}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<std::uint8_t> saturating_sub(const simd8<std::uint8_t> other) const
	{
		return _mm256_subs_epu8(*this, other);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<std::uint8_t> min_val(const simd8<std::uint8_t> other) const
	{
		return _mm256_min_epu8(other, *this);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<std::uint8_t> gt_bits(const simd8<std::uint8_t> other) const
	{
		return this->saturating_sub(other);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<bool> operator >=(const simd8<std::uint8_t> other) const
	{
		return other.min_val(*this) == other;
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE bool is_ascii() const
	{
		return _mm256_movemask_epi8(*this) == 0;
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE bool bits_not_set_anywhere() const
	{
		return _mm256_testz_si256(*this, *this) != 0;
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE bool any_bits_set_anywhere() const
	{
		return !bits_not_set_anywhere();
	}

	template <int N>
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<std::uint8_t> shr() const
	{
		return simd8<std::uint8_t>(_mm256_srli_epi16(*this, N)) & std::uint8_t(0xFFu >> N);
	}
};

template <typename T>
struct simd8x64
{
	static constexpr int NUM_CHUNKS = 2;
	simd8<T> chunks[NUM_CHUNKS];

	simd8x64(const simd8x64<T>&) = delete;
	simd8x64<T>& operator =(const simd8<T>) = delete;
	simd8x64() = delete;

	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8x64(const simd8<T> chunk0, const simd8<T> chunk1) : chunks{ chunk0, chunk1 } {}
	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8x64(const T* ptr)
		: chunks{ simd8<T>::load(ptr), simd8<T>::load(ptr + sizeof(simd8<T>) / sizeof(T)) }
	{}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE simd8<T> reduce_or() const
	{
		return this->chunks[0] | this->chunks[1];
	}
};

} // namespace simd

template <std::size_t STEP_SIZE>
struct buf_block_reader
{
	const std::uint8_t* buf;
	const std::size_t len;
	const std::size_t lenminusstep;
	std::size_t idx;

	UTF8_RANGES_SIMDUTF_REALLY_INLINE buf_block_reader(const std::uint8_t* _buf, std::size_t _len)
		: buf{ _buf }
		, len{ _len }
		, lenminusstep{ len < STEP_SIZE ? 0 : len - STEP_SIZE }
		, idx{ 0 }
	{}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE bool has_full_block() const { return idx < lenminusstep; }
	UTF8_RANGES_SIMDUTF_REALLY_INLINE const std::uint8_t* full_block() const { return &buf[idx]; }

	UTF8_RANGES_SIMDUTF_REALLY_INLINE std::size_t get_remainder(std::uint8_t* dst) const
	{
		if (len == idx)
		{
			return 0;
		}
		std::memset(dst, 0x20, STEP_SIZE);
		std::memcpy(dst, buf + idx, len - idx);
		return len - idx;
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE void advance() { idx += STEP_SIZE; }
};

UTF8_RANGES_SIMDUTF_REALLY_INLINE bool is_ascii(const simd::simd8x64<std::uint8_t>& input)
{
	return input.reduce_or().is_ascii();
}

UTF8_RANGES_SIMDUTF_REALLY_INLINE simd::simd8<bool> must_be_2_3_continuation(
	const simd::simd8<std::uint8_t> prev2,
	const simd::simd8<std::uint8_t> prev3)
{
	simd::simd8<std::uint8_t> is_third_byte = prev2.saturating_sub(0xe0u - 0x80);
	simd::simd8<std::uint8_t> is_fourth_byte = prev3.saturating_sub(0xf0u - 0x80);
	return simd::simd8<bool>(is_third_byte | is_fourth_byte);
}

UTF8_RANGES_SIMDUTF_REALLY_INLINE simd::simd8<std::uint8_t> check_special_cases(
	const simd::simd8<std::uint8_t> input,
	const simd::simd8<std::uint8_t> prev1)
{
	using simd::simd8;
	constexpr const std::uint8_t TOO_SHORT = 1 << 0;
	constexpr const std::uint8_t TOO_LONG = 1 << 1;
	constexpr const std::uint8_t OVERLONG_3 = 1 << 2;
	constexpr const std::uint8_t TOO_LARGE = 1 << 3;
	constexpr const std::uint8_t SURROGATE = 1 << 4;
	constexpr const std::uint8_t OVERLONG_2 = 1 << 5;
	constexpr const std::uint8_t TOO_LARGE_1000 = 1 << 6;
	constexpr const std::uint8_t OVERLONG_4 = 1 << 6;
	constexpr const std::uint8_t TWO_CONTS = 1 << 7;

	const simd8<std::uint8_t> byte_1_high = prev1.shr<4>().template lookup_16<std::uint8_t>(
		TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG, TOO_LONG,
		TWO_CONTS, TWO_CONTS, TWO_CONTS, TWO_CONTS,
		TOO_SHORT | OVERLONG_2,
		TOO_SHORT,
		TOO_SHORT | OVERLONG_3 | SURROGATE,
		TOO_SHORT | TOO_LARGE | TOO_LARGE_1000 | OVERLONG_4);

	constexpr const std::uint8_t CARRY = TOO_SHORT | TOO_LONG | TWO_CONTS;
	const simd8<std::uint8_t> byte_1_low = (prev1 & 0x0F).template lookup_16<std::uint8_t>(
		CARRY | OVERLONG_3 | OVERLONG_2 | OVERLONG_4,
		CARRY | OVERLONG_2,
		CARRY, CARRY,
		CARRY | TOO_LARGE,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000 | SURROGATE,
		CARRY | TOO_LARGE | TOO_LARGE_1000,
		CARRY | TOO_LARGE | TOO_LARGE_1000);

	const simd8<std::uint8_t> byte_2_high = input.shr<4>().template lookup_16<std::uint8_t>(
		TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT,
		TOO_LONG | OVERLONG_2 | TWO_CONTS | OVERLONG_3 | TOO_LARGE_1000 | OVERLONG_4,
		TOO_LONG | OVERLONG_2 | TWO_CONTS | OVERLONG_3 | TOO_LARGE,
		TOO_LONG | OVERLONG_2 | TWO_CONTS | SURROGATE | TOO_LARGE,
		TOO_LONG | OVERLONG_2 | TWO_CONTS | SURROGATE | TOO_LARGE,
		TOO_SHORT, TOO_SHORT, TOO_SHORT, TOO_SHORT);

	return byte_1_high & byte_1_low & byte_2_high;
}

UTF8_RANGES_SIMDUTF_REALLY_INLINE simd::simd8<std::uint8_t> check_multibyte_lengths(
	const simd::simd8<std::uint8_t> input,
	const simd::simd8<std::uint8_t> prev_input,
	const simd::simd8<std::uint8_t> sc)
{
	simd::simd8<std::uint8_t> prev2 = input.template prev<2>(prev_input);
	simd::simd8<std::uint8_t> prev3 = input.template prev<3>(prev_input);
	simd::simd8<std::uint8_t> must23 = simd::simd8<std::uint8_t>(must_be_2_3_continuation(prev2, prev3));
	simd::simd8<std::uint8_t> must23_80 = must23 & std::uint8_t(0x80);
	return must23_80 ^ sc;
}

UTF8_RANGES_SIMDUTF_REALLY_INLINE simd::simd8<std::uint8_t> is_incomplete(const simd::simd8<std::uint8_t> input)
{
	static const std::uint8_t max_array[32] = {
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 0b11110000u - 1, 0b11100000u - 1, 0b11000000u - 1
	};
	const simd::simd8<std::uint8_t> max_value(&max_array[0]);
	return input.gt_bits(max_value);
}

struct utf8_checker
{
	simd::simd8<std::uint8_t> error{};
	simd::simd8<std::uint8_t> prev_input_block{};
	simd::simd8<std::uint8_t> prev_incomplete{};

	UTF8_RANGES_SIMDUTF_REALLY_INLINE void check_utf8_bytes(
		const simd::simd8<std::uint8_t> input,
		const simd::simd8<std::uint8_t> prev_input)
	{
		simd::simd8<std::uint8_t> prev1 = input.template prev<1>(prev_input);
		simd::simd8<std::uint8_t> sc = check_special_cases(input, prev1);
		this->error |= check_multibyte_lengths(input, prev_input, sc);
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE void check_eof()
	{
		this->error |= this->prev_incomplete;
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE void check_next_input(const simd::simd8x64<std::uint8_t>& input)
	{
		if (UTF8_RANGES_SIMDUTF_LIKELY(is_ascii(input)))
		{
			this->error |= this->prev_incomplete;
		}
		else
		{
			this->check_utf8_bytes(input.chunks[0], this->prev_input_block);
			this->check_utf8_bytes(input.chunks[1], input.chunks[0]);
			this->prev_incomplete = is_incomplete(input.chunks[simd::simd8x64<std::uint8_t>::NUM_CHUNKS - 1]);
			this->prev_input_block = input.chunks[simd::simd8x64<std::uint8_t>::NUM_CHUNKS - 1];
		}
	}

	UTF8_RANGES_SIMDUTF_REALLY_INLINE bool errors() const
	{
		return this->error.any_bits_set_anywhere();
	}
};

template <class Checker>
UTF8_RANGES_SIMDUTF_REALLY_INLINE bool generic_validate_utf8(const std::uint8_t* input, std::size_t length)
{
	Checker c{};
	buf_block_reader<64> reader(input, length);
	while (reader.has_full_block())
	{
		simd::simd8x64<std::uint8_t> in(reader.full_block());
		c.check_next_input(in);
		reader.advance();
	}
	std::uint8_t block[64]{};
	reader.get_remainder(block);
	simd::simd8x64<std::uint8_t> in(block);
	c.check_next_input(in);
	reader.advance();
	c.check_eof();
	return !c.errors();
}

inline bool validate_utf8(const char* input, std::size_t length) noexcept
{
	return generic_validate_utf8<utf8_checker>(reinterpret_cast<const std::uint8_t*>(input), length);
}

#else

inline bool validate_utf8(const char*, std::size_t) noexcept
{
	return false;
}

#endif

} // namespace unicode_ranges::details::internal::simdutf_haswell_validation

UTF8_RANGES_SIMDUTF_UNTARGET_REGION

#undef UTF8_RANGES_SIMDUTF_REALLY_INLINE
#undef UTF8_RANGES_SIMDUTF_LIKELY
#undef UTF8_RANGES_SIMDUTF_TARGET_REGION
#undef UTF8_RANGES_SIMDUTF_UNTARGET_REGION

#endif // UTF8_RANGES_INTERNAL_SIMDUTF_HASWELL_UTF8_VALIDATE_HPP
