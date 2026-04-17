#include <array>
#include <cstdint>
#include <vector>

#include "unicode_ranges.hpp"

using namespace unicode_ranges;

struct strict_ascii_encoder
{
	using code_unit_type = char8_t;

	enum class encode_error
	{
		unrepresentable_scalar
	};

	static constexpr bool allow_implicit_construction = true;

	template <typename Writer>
	constexpr auto encode_one(char32_t scalar, Writer out) -> std::expected<void, encode_error>
	{
		if (scalar > 0x7Fu)
		{
			return std::unexpected(encode_error::unrepresentable_scalar);
		}

		out.push(static_cast<char8_t>(scalar));
		return {};
	}
};

struct strict_ascii_decoder
{
	using code_unit_type = char8_t;

	enum class decode_error
	{
		invalid_input
	};

	static constexpr bool allow_implicit_construction = true;

	template <typename Writer>
	constexpr auto decode_one(std::basic_string_view<char8_t> input, Writer out)
		-> std::expected<std::size_t, decode_error>
	{
		const auto byte = static_cast<std::uint8_t>(input.front());
		if (byte > 0x7Fu)
		{
			return std::unexpected(decode_error::invalid_input);
		}

		out.push(static_cast<char32_t>(byte));
		return 1;
	}
};

int main()
{
	auto decoded = utf8_string::from_encoded<strict_ascii_decoder>(u8"Hello");
	if (!decoded || decoded->base() != u8"Hello")
	{
		return 1;
	}

	auto owned_bytes = decoded->to_encoded<strict_ascii_encoder>();
	if (!owned_bytes || *owned_bytes != u8"Hello")
	{
		return 1;
	}

	std::array<char8_t, 5> bounded{};
	auto bounded_result = decoded->encode_to<strict_ascii_encoder>(std::span<char8_t>{ bounded });
	if (!bounded_result || std::u8string_view{ bounded.data(), bounded.size() } != u8"Hello")
	{
		return 1;
	}

	std::vector<char8_t> appended{ static_cast<char8_t>('>') };
	auto append_result = decoded->encode_append_to<strict_ascii_encoder>(appended);
	if (!append_result || appended != std::vector<char8_t>{
		static_cast<char8_t>('>'),
		static_cast<char8_t>('H'),
		static_cast<char8_t>('e'),
		static_cast<char8_t>('l'),
		static_cast<char8_t>('l'),
		static_cast<char8_t>('o') })
	{
		return 1;
	}

	return 0;
}
