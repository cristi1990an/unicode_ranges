#include <array>
#include <vector>

#include "unicode_ranges.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	const std::array<char8_t, 8> windows_bytes{
		static_cast<char8_t>('P'),
		static_cast<char8_t>('r'),
		static_cast<char8_t>('i'),
		static_cast<char8_t>('c'),
		static_cast<char8_t>('e'),
		static_cast<char8_t>(':'),
		static_cast<char8_t>(' '),
		static_cast<char8_t>(0x80u)
	};

	const auto windows_text = utf8_string::from_encoded<encodings::windows_1252>(
		std::u8string_view{ windows_bytes.data(), windows_bytes.size() });
	if (windows_text.base() != u8"Price: \u20AC")
	{
		return 1;
	}

	const auto windows_round_trip = windows_text.to_encoded<encodings::windows_1252>();
	if (!windows_round_trip || *windows_round_trip != std::u8string{ windows_bytes.begin(), windows_bytes.end() })
	{
		return 1;
	}

	const auto strict_ascii = utf8_string::from_encoded<encodings::ascii_strict>(u8"Hello");
	if (!strict_ascii || strict_ascii->base() != u8"Hello")
	{
		return 1;
	}

	encodings::ascii_lossy lossy{};
	std::vector<char8_t> lossy_bytes{};
	u8"Caf\u00E9"_utf8_sv.to_utf8_owned().encode_append_to(lossy_bytes, lossy);
	if (lossy.replacement_count != 1 || lossy_bytes != std::vector<char8_t>{
		static_cast<char8_t>('C'),
		static_cast<char8_t>('a'),
		static_cast<char8_t>('f'),
		static_cast<char8_t>('?') })
	{
		return 1;
	}

	return 0;
}
