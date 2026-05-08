#include "unicode_ranges_all.hpp"

#include <cassert>
#include <concepts>
#include <ranges>
#include <utility>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

namespace
{

template <typename Text>
concept temporary_split_once_is_available =
	requires
	{
		std::declval<Text>().split_once(u8"="_u8c);
	};

template <typename Text>
concept temporary_grapheme_at_is_available =
	requires
	{
		std::declval<Text>().grapheme_at(0);
	};

} // namespace

int main()
{
	constexpr auto borrowed = u8"  caf\u00E9  "_utf8_sv;
	static_assert(std::same_as<decltype(borrowed.trim()), utf8_string_view>);
	static_assert(borrowed.trim() == u8"caf\u00E9"_utf8_sv);

	utf8_string text = u8"  caf\u00E9  "_utf8_s;
	auto copied = text.trim();
	static_assert(std::same_as<decltype(copied), utf8_string>);
	assert(copied == u8"caf\u00E9"_utf8_sv);
	assert(text == u8"  caf\u00E9  "_utf8_sv);

	auto disposable = u8"  caf\u00E9  "_utf8_s;
	auto trimmed = std::move(disposable).trim();
	static_assert(noexcept(std::move(std::declval<utf8_string&>()).trim()));
	assert(trimmed == u8"caf\u00E9"_utf8_sv);

	auto framed = u8"<<<payload>>>"_utf8_s;
	auto stripped = std::move(framed).strip_circumfix(u8"<<<"_utf8_sv, u8">>>"_utf8_sv);
	assert(stripped.has_value());
	assert(*stripped == u8"payload"_utf8_sv);

	auto suffix_source = u8"prefix/suffix"_utf8_s;
	auto suffix = std::move(suffix_source).substr(7);
	assert(suffix.has_value());
	assert(*suffix == u8"suffix"_utf8_sv);

	auto iterated = u8"caf\u00E9"_utf8_s;
	auto owned_chars = std::move(iterated).chars();
	static_assert(!std::copy_constructible<decltype(owned_chars)>);
	static_assert(!std::ranges::borrowed_range<decltype(owned_chars)>);
	assert(std::ranges::distance(owned_chars) == 4);

	utf8_string key_value = u8"name=value"_utf8_s;
	auto pair = key_value.split_once(u8"="_u8c);
	assert(pair.has_value());
	assert(pair.left() == u8"name"_utf8_sv);
	assert(pair.right() == u8"value"_utf8_sv);

	static_assert(!temporary_split_once_is_available<utf8_string>);
	static_assert(!temporary_split_once_is_available<const utf8_string>);
	static_assert(!temporary_grapheme_at_is_available<utf8_string>);
	static_assert(!temporary_grapheme_at_is_available<const utf8_string>);
}
