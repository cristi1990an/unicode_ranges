// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright (c) 2026 unicode_ranges contributors
//
// Provenance:
// - Adapted from: simdutf
// - Original files:
//   - src/haswell/avx2_convert_utf8_to_utf16.cpp
//   - src/generic/utf8_to_utf16/valid_utf8_to_utf16.h
//   - src/tables/utf8_to_utf16_tables.h
// - Upstream version: v7.7.0
// - Original license: MIT OR Apache-2.0
// - Changes for unicode_ranges:
//   - reduced to runtime AVX2 valid UTF-8 to UTF-16 conversion
//   - fixed to native-endian UTF-16 output only
//   - replaced simdutf scalar tail with a local valid-UTF-8 tail loop

#ifndef UTF8_RANGES_INTERNAL_SIMDUTF_HASWELL_UTF8_TO_UTF16_HPP
#define UTF8_RANGES_INTERNAL_SIMDUTF_HASWELL_UTF8_TO_UTF16_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "simdutf_haswell_utf8_validate.hpp"
#include "simdutf_utf8_to_utf16_tables.hpp"

#if (defined(__x86_64__) || defined(_M_AMD64)) && defined(__clang__)
	#define UTF8_RANGES_SIMDUTF_UTF16_TARGET_REGION                               \
		_Pragma("clang attribute push(__attribute__((target(\"avx2,popcnt\"))), apply_to=function)")
	#define UTF8_RANGES_SIMDUTF_UTF16_UNTARGET_REGION _Pragma("clang attribute pop")
#elif (defined(__x86_64__) || defined(_M_AMD64)) && defined(__GNUC__)
	#define UTF8_RANGES_SIMDUTF_UTF16_TARGET_REGION                               \
		_Pragma("GCC push_options") _Pragma("GCC target(\"avx2,popcnt\")")
	#define UTF8_RANGES_SIMDUTF_UTF16_UNTARGET_REGION _Pragma("GCC pop_options")
#else
	#define UTF8_RANGES_SIMDUTF_UTF16_TARGET_REGION
	#define UTF8_RANGES_SIMDUTF_UTF16_UNTARGET_REGION
#endif

UTF8_RANGES_SIMDUTF_UTF16_TARGET_REGION

namespace unicode_ranges::details::internal::simdutf_haswell_utf8_to_utf16
{

#if defined(__x86_64__) || defined(_M_AMD64)

using namespace simdutf_haswell_validation::simd;

inline std::size_t convert_masked_utf8_to_utf16(
	const char* input,
	std::uint64_t utf8_end_of_code_point_mask,
	char16_t*& utf16_output) noexcept
{
	const __m128i in = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
	const std::uint16_t input_utf8_end_of_code_point_mask =
		static_cast<std::uint16_t>(utf8_end_of_code_point_mask & 0x0FFFu);

	if (utf8_end_of_code_point_mask == 0x0FFFu)
	{
		__m256i ascii = _mm256_cvtepu8_epi16(in);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(utf16_output), ascii);
		utf16_output += 12;
		return 12;
	}

	if ((utf8_end_of_code_point_mask & 0xFFFFu) == 0xAAAau)
	{
		const __m128i sh = _mm_setr_epi8(1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14);
		const __m128i perm = _mm_shuffle_epi8(in, sh);
		const __m128i ascii = _mm_and_si128(perm, _mm_set1_epi16(0x7F));
		const __m128i highbyte = _mm_and_si128(perm, _mm_set1_epi16(0x1F00));
		const __m128i composed = _mm_or_si128(ascii, _mm_srli_epi16(highbyte, 2));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(utf16_output), composed);
		utf16_output += 8;
		return 16;
	}

	if (input_utf8_end_of_code_point_mask == 0x924u)
	{
		const __m128i sh = _mm_setr_epi8(2, 1, 0, -1, 5, 4, 3, -1, 8, 7, 6, -1, 11, 10, 9, -1);
		const __m128i perm = _mm_shuffle_epi8(in, sh);
		const __m128i ascii = _mm_and_si128(perm, _mm_set1_epi32(0x7F));
		const __m128i middlebyte = _mm_and_si128(perm, _mm_set1_epi32(0x3F00));
		const __m128i middlebyte_shifted = _mm_srli_epi32(middlebyte, 2);
		const __m128i highbyte = _mm_and_si128(perm, _mm_set1_epi32(0x0F0000));
		const __m128i highbyte_shifted = _mm_srli_epi32(highbyte, 4);
		const __m128i composed = _mm_or_si128(_mm_or_si128(ascii, middlebyte_shifted), highbyte_shifted);
		const __m128i composed_repacked = _mm_packus_epi32(composed, composed);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(utf16_output), composed_repacked);
		utf16_output += 4;
		return 12;
	}

	const std::uint8_t idx =
		simdutf_utf8_to_utf16_tables::utf8bigindex[input_utf8_end_of_code_point_mask][0];
	const std::uint8_t consumed =
		simdutf_utf8_to_utf16_tables::utf8bigindex[input_utf8_end_of_code_point_mask][1];

	if (idx < 64)
	{
		const __m128i sh = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(simdutf_utf8_to_utf16_tables::shufutf8[idx]));
		const __m128i perm = _mm_shuffle_epi8(in, sh);
		const __m128i ascii = _mm_and_si128(perm, _mm_set1_epi16(0x7F));
		const __m128i highbyte = _mm_and_si128(perm, _mm_set1_epi16(0x1F00));
		const __m128i composed = _mm_or_si128(ascii, _mm_srli_epi16(highbyte, 2));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(utf16_output), composed);
		utf16_output += 6;
	}
	else if (idx < 145)
	{
		const __m128i sh = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(simdutf_utf8_to_utf16_tables::shufutf8[idx]));
		const __m128i perm = _mm_shuffle_epi8(in, sh);
		const __m128i ascii = _mm_and_si128(perm, _mm_set1_epi32(0x7F));
		const __m128i middlebyte = _mm_and_si128(perm, _mm_set1_epi32(0x3F00));
		const __m128i middlebyte_shifted = _mm_srli_epi32(middlebyte, 2);
		const __m128i highbyte = _mm_and_si128(perm, _mm_set1_epi32(0x0F0000));
		const __m128i highbyte_shifted = _mm_srli_epi32(highbyte, 4);
		const __m128i composed = _mm_or_si128(_mm_or_si128(ascii, middlebyte_shifted), highbyte_shifted);
		const __m128i composed_repacked = _mm_packus_epi32(composed, composed);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(utf16_output), composed_repacked);
		utf16_output += 4;
	}
	else if (idx < 209)
	{
		const __m128i sh = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(simdutf_utf8_to_utf16_tables::shufutf8[idx]));
		const __m128i perm = _mm_shuffle_epi8(in, sh);
		const __m128i ascii = _mm_and_si128(perm, _mm_set1_epi32(0x7F));
		const __m128i middlebyte = _mm_and_si128(perm, _mm_set1_epi32(0x3F00));
		const __m128i middlebyte_shifted = _mm_srli_epi32(middlebyte, 2);
		__m128i middlehighbyte = _mm_and_si128(perm, _mm_set1_epi32(0x3F0000));
		const __m128i correct = _mm_srli_epi32(_mm_and_si128(perm, _mm_set1_epi32(0x400000)), 1);
		middlehighbyte = _mm_xor_si128(correct, middlehighbyte);
		const __m128i middlehighbyte_shifted = _mm_srli_epi32(middlehighbyte, 4);
		const __m128i highbyte = _mm_and_si128(perm, _mm_set1_epi32(0xFF000000));
		const __m128i highbyte_shifted = _mm_srli_epi32(highbyte, 6);
		const __m128i composed = _mm_or_si128(
			_mm_or_si128(ascii, middlebyte_shifted),
			_mm_or_si128(highbyte_shifted, middlehighbyte_shifted));
		const __m128i composedminus = _mm_sub_epi32(composed, _mm_set1_epi32(0x10000));
		const __m128i lowtenbits = _mm_and_si128(composedminus, _mm_set1_epi32(0x3FF));
		const __m128i hightenbits = _mm_and_si128(_mm_srli_epi32(composedminus, 10), _mm_set1_epi32(0x3FF));
		const __m128i lowtenbitsadd = _mm_add_epi32(lowtenbits, _mm_set1_epi32(0xDC00));
		const __m128i hightenbitsadd = _mm_add_epi32(hightenbits, _mm_set1_epi32(0xD800));
		const __m128i lowtenbitsaddshifted = _mm_slli_epi32(lowtenbitsadd, 16);
		const __m128i surrogates = _mm_or_si128(hightenbitsadd, lowtenbitsaddshifted);

		std::uint32_t basic_buffer[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(basic_buffer), composed);
		std::uint32_t surrogate_buffer[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(surrogate_buffer), surrogates);
		for (std::size_t i = 0; i < 3; ++i)
		{
			if (basic_buffer[i] > 0x3C00000)
			{
				utf16_output[0] = static_cast<char16_t>(surrogate_buffer[i] & 0xFFFFu);
				utf16_output[1] = static_cast<char16_t>(surrogate_buffer[i] >> 16);
				utf16_output += 2;
			}
			else
			{
				utf16_output[0] = static_cast<char16_t>(basic_buffer[i]);
				++utf16_output;
			}
		}
	}

	return consumed;
}

inline std::uint32_t decode_valid_utf8_scalar(const char* bytes, std::size_t count) noexcept
{
	const auto* data = reinterpret_cast<const std::uint8_t*>(bytes);
	switch (count)
	{
	case 1:
		return data[0];
	case 2:
		return ((data[0] & 0x1Fu) << 6)
			| (data[1] & 0x3Fu);
	case 3:
		return ((data[0] & 0x0Fu) << 12)
			| ((data[1] & 0x3Fu) << 6)
			| (data[2] & 0x3Fu);
	default:
		return ((data[0] & 0x07u) << 18)
			| ((data[1] & 0x3Fu) << 12)
			| ((data[2] & 0x3Fu) << 6)
			| (data[3] & 0x3Fu);
	}
}

inline std::size_t encode_utf16_scalar(std::uint32_t scalar, char16_t* out) noexcept
{
	if (scalar <= 0xFFFFu)
	{
		out[0] = static_cast<char16_t>(scalar);
		return 1;
	}

	scalar -= 0x10000u;
	out[0] = static_cast<char16_t>(0xD800u + ((scalar >> 10) & 0x3FFu));
	out[1] = static_cast<char16_t>(0xDC00u + (scalar & 0x3FFu));
	return 2;
}

inline std::size_t convert_valid(std::u8string_view bytes, char16_t* utf16_output) noexcept
{
	const char* input = reinterpret_cast<const char*>(bytes.data());
	const std::size_t size = bytes.size();
	std::size_t pos = 0;
	char16_t* start = utf16_output;
	const std::size_t safety_margin = 16;

	while (pos + 64 + safety_margin <= size)
	{
		simd8x64<std::int8_t> in(reinterpret_cast<const std::int8_t*>(input + pos));
		if (in.is_ascii())
		{
			in.store_ascii_as_utf16(utf16_output);
			utf16_output += 64;
			pos += 64;
		}
		else
		{
			const std::uint64_t utf8_continuation_mask = in.lt(static_cast<std::int8_t>(-64));
			const std::uint64_t utf8_leading_mask = ~utf8_continuation_mask;
			std::uint64_t utf8_end_of_code_point_mask = utf8_leading_mask >> 1;
			const std::size_t max_starting_point = (pos + 64) - 12;
			while (pos < max_starting_point)
			{
				const std::size_t consumed =
					convert_masked_utf8_to_utf16(input + pos, utf8_end_of_code_point_mask, utf16_output);
				pos += consumed;
				utf8_end_of_code_point_mask >>= consumed;
			}
		}
	}

	while (pos < size)
	{
		const std::uint8_t lead = static_cast<std::uint8_t>(input[pos]);
		if (lead < 0x80u)
		{
			*utf16_output++ = static_cast<char16_t>(lead);
			++pos;
			continue;
		}

		const std::size_t count = (lead & 0xE0u) == 0xC0u ? 2
			: (lead & 0xF0u) == 0xE0u ? 3
			: 4;
		const std::uint32_t scalar = decode_valid_utf8_scalar(input + pos, count);
		utf16_output += encode_utf16_scalar(scalar, utf16_output);
		pos += count;
	}

	return static_cast<std::size_t>(utf16_output - start);
}

#else

inline std::size_t convert_valid(std::u8string_view, char16_t*) noexcept
{
	return 0;
}

#endif

} // namespace unicode_ranges::details::internal::simdutf_haswell_utf8_to_utf16

UTF8_RANGES_SIMDUTF_UTF16_UNTARGET_REGION

#undef UTF8_RANGES_SIMDUTF_UTF16_TARGET_REGION
#undef UTF8_RANGES_SIMDUTF_UTF16_UNTARGET_REGION

#endif // UTF8_RANGES_INTERNAL_SIMDUTF_HASWELL_UTF8_TO_UTF16_HPP
