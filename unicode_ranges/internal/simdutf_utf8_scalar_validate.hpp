// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright (c) 2026 unicode_ranges contributors
//
// Provenance:
// - Adapted from: simdutf
// - Original file: src/scalar/utf8.h
// - Upstream version: v7.7.0
// - Original license: MIT OR Apache-2.0
// - Changes for unicode_ranges: reduced to runtime UTF-8 validation with
//   unicode_ranges error mapping and removed unrelated APIs.

#ifndef UTF8_RANGES_INTERNAL_SIMDUTF_UTF8_SCALAR_VALIDATE_HPP
#define UTF8_RANGES_INTERNAL_SIMDUTF_UTF8_SCALAR_VALIDATE_HPP

#include <cstdint>
#include <cstring>
#include <expected>
#include <string_view>

namespace unicode_ranges::details::internal
{

template <typename CharT>
requires (sizeof(CharT) == 1)
inline std::expected<void, utf8_error> simdutf_scalar_validate_utf8(
	std::basic_string_view<CharT> value) noexcept
{
	const auto* data = reinterpret_cast<const std::uint8_t*>(value.data());
	const auto len = value.size();

	std::size_t pos = 0;
	std::uint32_t code_point = 0;
	while (pos < len)
	{
		std::size_t next_pos = pos + 16;
		if (next_pos <= len)
		{
			std::uint64_t v1;
			std::memcpy(&v1, data + pos, sizeof(v1));
			std::uint64_t v2;
			std::memcpy(&v2, data + pos + sizeof(v1), sizeof(v2));
			const std::uint64_t v = v1 | v2;
			if ((v & 0x8080808080808080ull) == 0)
			{
				pos = next_pos;
				continue;
			}
		}

		unsigned char byte = data[pos];
		while (byte < 0b10000000)
		{
			if (++pos == len)
			{
				return {};
			}
			byte = data[pos];
		}

		if ((byte & 0b11100000) == 0b11000000)
		{
			next_pos = pos + 2;
			if (next_pos > len)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = pos
				});
			}
			if ((data[pos + 1] & 0b11000000) != 0b10000000)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = pos
				});
			}

			code_point = (byte & 0b00011111) << 6
				| (data[pos + 1] & 0b00111111);
			if ((code_point < 0x80) || (0x7ff < code_point))
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::invalid_sequence,
					.first_invalid_byte_index = pos
				});
			}
		}
		else if ((byte & 0b11110000) == 0b11100000)
		{
			next_pos = pos + 3;
			if (next_pos > len)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = pos
				});
			}
			if ((data[pos + 1] & 0b11000000) != 0b10000000
				|| (data[pos + 2] & 0b11000000) != 0b10000000)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = pos
				});
			}

			code_point = (byte & 0b00001111) << 12
				| (data[pos + 1] & 0b00111111) << 6
				| (data[pos + 2] & 0b00111111);
			if ((code_point < 0x800) || (0xffff < code_point))
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::invalid_sequence,
					.first_invalid_byte_index = pos
				});
			}
			if (0xd7ff < code_point && code_point < 0xe000)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::invalid_sequence,
					.first_invalid_byte_index = pos
				});
			}
		}
		else if ((byte & 0b11111000) == 0b11110000)
		{
			next_pos = pos + 4;
			if (next_pos > len)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = pos
				});
			}
			if ((data[pos + 1] & 0b11000000) != 0b10000000
				|| (data[pos + 2] & 0b11000000) != 0b10000000
				|| (data[pos + 3] & 0b11000000) != 0b10000000)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::truncated_sequence,
					.first_invalid_byte_index = pos
				});
			}

			code_point =
				(byte & 0b00000111) << 18
				| (data[pos + 1] & 0b00111111) << 12
				| (data[pos + 2] & 0b00111111) << 6
				| (data[pos + 3] & 0b00111111);
			if (code_point <= 0xffff || 0x10ffff < code_point)
			{
				return std::unexpected(utf8_error{
					.code = utf8_error_code::invalid_sequence,
					.first_invalid_byte_index = pos
				});
			}
		}
		else
		{
			return std::unexpected(utf8_error{
				.code = ((byte & 0b11000000) == 0b10000000)
					? utf8_error_code::invalid_sequence
					: utf8_error_code::invalid_lead_byte,
				.first_invalid_byte_index = pos
			});
		}

		pos = next_pos;
	}

	return {};
}

} // namespace unicode_ranges::details::internal

#endif // UTF8_RANGES_INTERNAL_SIMDUTF_UTF8_SCALAR_VALIDATE_HPP
