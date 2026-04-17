#include <array>
#include <vector>

#include "unicode_ranges.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto decoded = utf8_string::from_encoded<encodings::ascii_strict>(u8"Hello");
	if (!decoded)
	{
		return 1;
	}

	auto strict_bytes = decoded->to_encoded<encodings::ascii_strict>();
	if (!strict_bytes)
	{
		return 1;
	}

	std::array<char8_t, 5> bounded{};
	encodings::ascii_strict strict{};
	auto wrote_bounded = decoded->encode_to(std::span<char8_t>{ bounded }, strict);
	if (!wrote_bounded)
	{
		return 1;
	}

	const std::array<char8_t, 8> windows_input{
		static_cast<char8_t>('P'),
		static_cast<char8_t>('r'),
		static_cast<char8_t>('i'),
		static_cast<char8_t>('c'),
		static_cast<char8_t>('e'),
		static_cast<char8_t>(':'),
		static_cast<char8_t>(' '),
		static_cast<char8_t>(0x80u)
	};
	auto windows = utf8_string::from_encoded<encodings::windows_1252>(
		std::u8string_view{ windows_input.data(), windows_input.size() });
	auto windows_encoded = windows.to_encoded<encodings::windows_1252>();
	if (!windows_encoded || *windows_encoded != std::u8string{ windows_input.begin(), windows_input.end() })
	{
		return 1;
	}

	std::vector<char8_t> lossy_bytes{ static_cast<char8_t>('>') };
	encodings::ascii_lossy lossy{};
	u8"Caf\u00E9"_utf8_sv.to_utf8_owned().encode_append_to(lossy_bytes, lossy);

	return windows.base() == u8"Price: \u20AC" && lossy.replacement_count == 1 ? 0 : 1;
}
