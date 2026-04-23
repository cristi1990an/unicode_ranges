#include <vector>

#include "unicode_ranges_all.hpp"

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

struct counting_ascii_lossy_encoder
{
	using code_unit_type = char8_t;

	static constexpr bool allow_implicit_construction = false;

	std::size_t replacement_count = 0;

	template <typename Writer>
	constexpr void encode_one(char32_t scalar, Writer out)
	{
		if (scalar <= 0x7Fu)
		{
			out.push(static_cast<char8_t>(scalar));
			return;
		}

		out.push(static_cast<char8_t>('?'));
		++replacement_count;
	}
};

int main()
{
	counting_ascii_lossy_encoder encoder{};
	std::vector<char8_t> bytes{};
	u8"Caf\u00E9"_utf8_sv.to_utf8_owned().encode_append_to(bytes, encoder);

	return encoder.replacement_count == 1
		&& bytes == std::vector<char8_t>{
			static_cast<char8_t>('C'),
			static_cast<char8_t>('a'),
			static_cast<char8_t>('f'),
			static_cast<char8_t>('?') }
		? 0
		: 1;
}
