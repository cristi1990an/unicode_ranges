#include "unicode_ranges_all.hpp"

#include <cassert>
#include <ranges>
#include <utility>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	utf8_string text = u8"  caf\u00E9  "_utf8_s;

	auto copied = text.trim();
	assert(copied == u8"caf\u00E9"_utf8_sv);
	assert(text == u8"  caf\u00E9  "_utf8_sv);

	auto disposable = u8"  caf\u00E9  "_utf8_s;
	auto trimmed = std::move(disposable).trim();
	assert(trimmed == u8"caf\u00E9"_utf8_sv);

	auto framed = u8"<<<payload>>>"_utf8_s;
	auto stripped = std::move(framed).strip_circumfix(u8"<<<"_utf8_sv, u8">>>"_utf8_sv);
	assert(stripped.has_value());
	assert(*stripped == u8"payload"_utf8_sv);

	utf8_string key_value = u8"name=value"_utf8_s;
	auto split = key_value.split_once(u8"="_u8c);
	assert(split.has_value());
	assert(split.left() == u8"name"_utf8_sv);
	assert(split.right() == u8"value"_utf8_sv);
	assert(std::ranges::size(split) == 2);
}
