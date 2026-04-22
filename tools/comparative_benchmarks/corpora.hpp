#ifndef UTF8_RANGES_COMPARATIVE_BENCHMARKS_CORPORA_HPP
#define UTF8_RANGES_COMPARATIVE_BENCHMARKS_CORPORA_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "model.hpp"

namespace comparative_benchmarks
{

inline std::u8string repeat_text(std::u8string_view chunk, std::size_t count)
{
	std::u8string result{};
	result.reserve(chunk.size() * count);
	for (std::size_t i = 0; i != count; ++i)
	{
		result.append(chunk);
	}

	return result;
}

inline const std::vector<corpus>& utf8_corpora()
{
	static const std::vector<corpus> value = [] {
		std::vector<corpus> corpora{};
		corpora.reserve(5);

		corpora.push_back(corpus{
			.id = "ascii_heavy",
			.description = "ASCII-heavy valid UTF-8 payload",
			.bytes = repeat_text(u8"ASCII payload 0123456789 ABCDEF ", 4096),
			.valid_utf8 = true
		});
		corpora.push_back(corpus{
			.id = "western_mixed",
			.description = "Western European text with accents and symbols",
			.bytes = repeat_text(u8"Café déjà vu Ångström naïve € ", 2048),
			.valid_utf8 = true
		});
		corpora.push_back(corpus{
			.id = "emoji_combining",
			.description = "Combining marks and emoji-heavy UTF-8 text",
			.bytes = repeat_text(u8"é 👩‍💻 🙂 🇷🇴 café ", 2048),
			.valid_utf8 = true
		});
		corpora.push_back(corpus{
			.id = "cyrillic_mixed",
			.description = "Cyrillic-heavy UTF-8 text with punctuation and symbols",
			.bytes = repeat_text(u8"Привет, мир! Ё ё № € ", 2048),
			.valid_utf8 = true
		});

		auto invalid = repeat_text(u8"invalid prefix ", 1024);
		invalid.push_back(static_cast<char8_t>(0x80u));
		invalid.push_back(static_cast<char8_t>('X'));
		invalid.append(u8" invalid suffix");
		corpora.push_back(corpus{
			.id = "invalid_utf8",
			.description = "UTF-8 payload with an isolated continuation byte",
			.bytes = std::move(invalid),
			.valid_utf8 = false
		});

		return corpora;
	}();

	return value;
}

} // namespace comparative_benchmarks

#endif // UTF8_RANGES_COMPARATIVE_BENCHMARKS_CORPORA_HPP
