#include <array>
#include <vector>

#include "unicode_ranges_full.hpp"

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

	const std::array<char8_t, 2> latin1_bytes{
		static_cast<char8_t>('C'),
		static_cast<char8_t>(0xE9u)
	};
	const auto latin1_text = utf8_string::from_encoded<encodings::iso_8859_1>(
		std::u8string_view{ latin1_bytes.data(), latin1_bytes.size() });
	if (latin1_text.base() != u8"C\u00E9")
	{
		return 1;
	}

	const auto latin1_round_trip = latin1_text.to_encoded<encodings::iso_8859_1>();
	if (!latin1_round_trip || *latin1_round_trip != std::u8string{ latin1_bytes.begin(), latin1_bytes.end() })
	{
		return 1;
	}

	const std::array<char8_t, 4> latin9_bytes{
		static_cast<char8_t>(0xA4u),
		static_cast<char8_t>(0xBCu),
		static_cast<char8_t>(0xBDu),
		static_cast<char8_t>(0xBEu)
	};
	const auto latin9_text = utf8_string::from_encoded<encodings::iso_8859_15>(
		std::u8string_view{ latin9_bytes.data(), latin9_bytes.size() });
	if (latin9_text.base() != u8"\u20AC\u0152\u0153\u0178")
	{
		return 1;
	}

	const auto latin9_round_trip = latin9_text.to_encoded<encodings::iso_8859_15>();
	if (!latin9_round_trip || *latin9_round_trip != std::u8string{ latin9_bytes.begin(), latin9_bytes.end() })
	{
		return 1;
	}

	const std::array<char8_t, 6> windows_1251_bytes{
		static_cast<char8_t>(0xCFu),
		static_cast<char8_t>(0xF0u),
		static_cast<char8_t>(0xE8u),
		static_cast<char8_t>(0xE2u),
		static_cast<char8_t>(0xE5u),
		static_cast<char8_t>(0xF2u)
	};
	const auto windows_1251_text = utf8_string::from_encoded<encodings::windows_1251>(
		std::u8string_view{ windows_1251_bytes.data(), windows_1251_bytes.size() });
	if (windows_1251_text.base() != u8"\u041F\u0440\u0438\u0432\u0435\u0442")
	{
		return 1;
	}

	const auto windows_1251_round_trip = windows_1251_text.to_encoded<encodings::windows_1251>();
	if (!windows_1251_round_trip || *windows_1251_round_trip != std::u8string{ windows_1251_bytes.begin(), windows_1251_bytes.end() })
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
