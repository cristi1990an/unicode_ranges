#include <cassert>
#include <array>
#include <vector>

#include "unicode_ranges_all.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	auto decoded = utf8_string::from_encoded<encodings::ascii_strict>(u8"Hello");
	assert(decoded);

	auto strict_bytes = decoded->to_encoded<encodings::ascii_strict>();
	assert(strict_bytes);
	assert(*strict_bytes == u8"Hello");

	std::array<char8_t, 5> bounded{};
	encodings::ascii_strict strict{};
	auto wrote_bounded = decoded->encode_to(std::span<char8_t>{ bounded }, strict);
	assert(wrote_bounded);
	const std::u8string_view bounded_view{ bounded.data(), bounded.size() };
	assert(bounded_view == u8"Hello");

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
	const auto windows = utf8_string::from_encoded<encodings::windows_1252>(
		std::u8string_view{ windows_input.data(), windows_input.size() });
	assert(windows.base() == u8"Price: \u20AC");
	auto windows_encoded = windows.to_encoded<encodings::windows_1252>();
	assert(windows_encoded);
	const std::u8string expected_windows_bytes{ windows_input.begin(), windows_input.end() };
	assert(*windows_encoded == expected_windows_bytes);

	std::vector<char8_t> lossy_bytes{ static_cast<char8_t>('>') };
	encodings::ascii_lossy lossy{};
	u8"Café"_utf8_sv.to_utf8_owned().encode_append_to(lossy_bytes, lossy);
	assert((lossy_bytes == std::vector<char8_t>{
		static_cast<char8_t>('>'),
		static_cast<char8_t>('C'),
		static_cast<char8_t>('a'),
		static_cast<char8_t>('f'),
		static_cast<char8_t>('?') }));
	assert(lossy.replacement_count == 1);

	return 0;
}
