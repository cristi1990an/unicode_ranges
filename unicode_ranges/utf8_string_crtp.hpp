#ifndef UTF8_RANGES_UTF8_STRING_CRTP_HPP
#define UTF8_RANGES_UTF8_STRING_CRTP_HPP

#include <span>

#include "utf8_views.hpp"

namespace unicode_ranges
{

namespace details
{

class utf8_char_indices_view : public std::ranges::view_interface<utf8_char_indices_view>
{
public:
	static constexpr utf8_char_indices_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(base).has_value());
		return utf8_char_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, utf8_char>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr reference operator*() const noexcept
		{
			const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base_[current_]));
			return { current_, utf8_char::from_utf8_bytes_unchecked(base_.data() + current_, count) };
		}

		constexpr iterator& operator++() noexcept
		{
			current_ += details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base_[current_]));
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == it.base_.size();
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == it.base_.size();
		}

	private:
		friend class utf8_char_indices_view;

		constexpr iterator(std::u8string_view base, std::size_t current) noexcept
			: base_(base), current_(current)
		{}

		std::u8string_view base_{};
		std::size_t current_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		return iterator{ base_, 0 };
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

	constexpr std::size_t reserve_hint() const noexcept
	{
		return base_.size();
	}

private:
	constexpr explicit utf8_char_indices_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

template <typename View>
class utf8_grapheme_indices_view : public std::ranges::view_interface<utf8_grapheme_indices_view<View>>
{
public:
	static constexpr utf8_grapheme_indices_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(base).has_value());
		return utf8_grapheme_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, View>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr reference operator*() const noexcept
		{
			return value_type{
				current_,
				View::from_bytes_unchecked(base_.substr(current_, next_ - current_))
			};
		}

		constexpr iterator& operator++() noexcept
		{
			current_ = next_;
			if (current_ != base_.size())
			{
				next_ = details::next_grapheme_boundary(base_, current_);
			}
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == it.base_.size();
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == it.base_.size();
		}

	private:
		friend class utf8_grapheme_indices_view<View>;

		constexpr iterator(std::u8string_view base, std::size_t current, std::size_t next) noexcept
			: base_(base), current_(current), next_(next)
		{}

		std::u8string_view base_{};
		std::size_t current_ = 0;
		std::size_t next_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		if (base_.empty())
		{
			return iterator{ base_, base_.size(), base_.size() };
		}

		return iterator{ base_, 0, details::next_grapheme_boundary(base_, 0) };
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

	constexpr std::size_t reserve_hint() const noexcept
	{
		return base_.size();
	}

private:
	constexpr explicit utf8_grapheme_indices_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

inline constexpr bool utf8_exact_match_at_scalar(
	std::u8string_view base,
	std::u8string_view needle,
	std::size_t pos) noexcept
{
	for (std::size_t needle_index = 0; needle_index != needle.size(); ++needle_index)
	{
		if (base[pos + needle_index] != needle[needle_index])
		{
			return false;
		}
	}

	return true;
}

inline bool utf8_exact_match_at_runtime(
	std::u8string_view base,
	std::u8string_view needle,
	std::size_t pos) noexcept
{
	return std::char_traits<char8_t>::compare(base.data() + pos, needle.data(), needle.size()) == 0;
}

class utf8_runtime_exact_searcher
{
public:
	explicit utf8_runtime_exact_searcher(std::u8string_view needle) noexcept
		: needle_(needle),
		  last_index_(needle.empty() ? 0u : needle.size() - 1u),
		  use_bmh_(needle.size() > 8u)
	{
		if (use_bmh_)
		{
			shift_.fill(needle_.size());
			for (std::size_t i = 0; i != last_index_; ++i)
			{
				shift_[static_cast<std::uint8_t>(needle_[i])] = last_index_ - i;
			}
		}
	}

	std::size_t find(std::u8string_view base, std::size_t pos) const noexcept
	{
		if (needle_.empty())
		{
			return pos <= base.size() ? pos : std::u8string_view::npos;
		}

		if (pos > base.size() || needle_.size() > base.size() - pos)
		{
			return std::u8string_view::npos;
		}

		if (needle_.size() == 1u)
		{
			return base.find(needle_.front(), pos);
		}

		if (!use_bmh_)
		{
			return find_prefilter(base, pos);
		}

		const auto limit = base.size() - needle_.size();
		while (pos <= limit)
		{
			const auto tail = pos + last_index_;
			const auto last = static_cast<std::uint8_t>(base[tail]);
			if (base[tail] == needle_[last_index_]
				&& utf8_exact_match_at_runtime(base, needle_, pos))
			{
				return pos;
			}

			pos += (std::max)(std::size_t{ 1 }, shift_[last]);
		}

		return std::u8string_view::npos;
	}

	std::size_t rfind(std::u8string_view base, std::size_t max_start) const noexcept
	{
		if (needle_.empty())
		{
			return (std::min)(base.size(), max_start);
		}

		if (needle_.size() > base.size())
		{
			return std::u8string_view::npos;
		}

		max_start = (std::min)(max_start, base.size() - needle_.size());
		if (needle_.size() == 1u)
		{
			return base.rfind(needle_.front(), max_start);
		}

		const auto last = needle_[last_index_];
		auto candidate_end = base.rfind(last, max_start + last_index_);
		while (candidate_end != std::u8string_view::npos)
		{
			if (candidate_end < last_index_)
			{
				break;
			}

			const auto candidate = candidate_end - last_index_;
			if (utf8_exact_match_at_runtime(base, needle_, candidate))
			{
				return candidate;
			}

			if (candidate_end == 0u)
			{
				break;
			}

			candidate_end = base.rfind(last, candidate_end - 1u);
		}

		return std::u8string_view::npos;
	}

private:
	std::size_t find_prefilter(std::u8string_view base, std::size_t pos) const noexcept
	{
		const auto limit = base.size() - needle_.size();
		const auto first = needle_.front();
		while (pos <= limit)
		{
			pos = base.find(first, pos);
			if (pos == std::u8string_view::npos || pos > limit)
			{
				return std::u8string_view::npos;
			}

			if (base[pos + last_index_] == needle_[last_index_]
				&& utf8_exact_match_at_runtime(base, needle_, pos))
			{
				return pos;
			}

			++pos;
		}

		return std::u8string_view::npos;
	}

	std::u8string_view needle_{};
	std::array<std::size_t, 256> shift_{};
	std::size_t last_index_ = 0;
	bool use_bmh_ = false;
};

inline constexpr std::size_t find_utf8_exact(
	std::u8string_view base,
	std::u8string_view needle,
	std::size_t pos) noexcept
{
	if (needle.empty())
	{
		return pos <= base.size() ? pos : std::u8string_view::npos;
	}

	if (pos > base.size() || needle.size() > base.size() - pos)
	{
		return std::u8string_view::npos;
	}

	if consteval
	{
		for (std::size_t index = pos; index + needle.size() <= base.size(); ++index)
		{
			if (utf8_exact_match_at_scalar(base, needle, index))
			{
				return index;
			}
		}

		return std::u8string_view::npos;
	}
	else
	{
		return utf8_runtime_exact_searcher{ needle }.find(base, pos);
	}
}

inline constexpr std::size_t rfind_utf8_exact(
	std::u8string_view base,
	std::u8string_view needle,
	std::size_t max_start) noexcept
{
	if (needle.empty())
	{
		return (std::min)(base.size(), max_start);
	}

	if (needle.size() > base.size())
	{
		return std::u8string_view::npos;
	}

	max_start = (std::min)(max_start, base.size() - needle.size());
	if consteval
	{
		for (std::size_t index = max_start + 1; index != 0; --index)
		{
			const auto candidate = index - 1;
			if (utf8_exact_match_at_scalar(base, needle, candidate))
			{
				return candidate;
			}
		}

		return std::u8string_view::npos;
	}
	else
	{
		return utf8_runtime_exact_searcher{ needle }.rfind(base, max_start);
	}
}

inline constexpr std::size_t find_utf8_split_delimiter(
	std::u8string_view base,
	std::u8string_view delimiter,
	std::size_t pos) noexcept
{
	if (delimiter.empty())
	{
		return std::u8string_view::npos;
	}

	if (pos > base.size() || delimiter.size() > base.size() - pos)
	{
		return std::u8string_view::npos;
	}

	return details::find_utf8_exact(base, delimiter, pos);
}

inline constexpr std::size_t rfind_utf8_split_delimiter(
	std::u8string_view base,
	std::u8string_view delimiter,
	std::size_t end_exclusive) noexcept
{
	if (delimiter.empty() || delimiter.size() > base.size() || end_exclusive < delimiter.size())
	{
		return std::u8string_view::npos;
	}

	const auto max_start = (std::min)(base.size() - delimiter.size(), end_exclusive - delimiter.size());
	return details::rfind_utf8_exact(base, delimiter, max_start);
}

inline constexpr bool utf8_split_input_ends_with_delimiter(
	std::u8string_view base,
	std::u8string_view delimiter) noexcept
{
	return !delimiter.empty()
		&& base.size() >= delimiter.size()
		&& base.substr(base.size() - delimiter.size()) == delimiter;
}

struct borrowed_utf8_split_delimiter
{
	std::u8string_view value{};

	constexpr borrowed_utf8_split_delimiter() = default;

	constexpr explicit borrowed_utf8_split_delimiter(std::u8string_view value) noexcept
		: value(value)
	{
	}

	constexpr std::u8string_view view() const noexcept
	{
		return value;
	}
};

struct owned_utf8_split_char_delimiter
{
	std::array<char8_t, 4> bytes{};
	std::uint8_t size = 0;

	constexpr owned_utf8_split_char_delimiter() = default;

	constexpr explicit owned_utf8_split_char_delimiter(utf8_char delimiter) noexcept
	{
		const auto delimiter_view = details::utf8_char_view(delimiter);
		size = static_cast<std::uint8_t>(delimiter_view.size());
		for (std::size_t i = 0; i != delimiter_view.size(); ++i)
		{
			bytes[i] = delimiter_view[i];
		}
	}

	constexpr std::u8string_view view() const noexcept
	{
		return std::u8string_view{ bytes.data(), size };
	}
};

template <typename Pred>
concept utf8_char_predicate
	= std::copy_constructible<std::remove_cvref_t<Pred>>
	&& std::predicate<const std::remove_cvref_t<Pred>&, utf8_char>;

struct utf8_char_span_matcher
{
	static constexpr std::size_t non_ascii_inline_capacity = 16;

	std::span<const utf8_char> chars{};
	std::array<std::uint64_t, 2> ascii_bits{};
	std::array<std::uint64_t, 4> non_ascii_lead_bits{};
	std::array<std::uint32_t, non_ascii_inline_capacity> non_ascii_scalars{};
	std::uint8_t non_ascii_count = 0;
	bool non_ascii_overflow = false;

	constexpr utf8_char_span_matcher() noexcept = default;

	constexpr explicit utf8_char_span_matcher(std::span<const utf8_char> chars) noexcept
		: chars(chars)
	{
		for (utf8_char ch : chars)
		{
			const auto scalar = ch.as_scalar();
			if (scalar <= encoding_constants::ascii_scalar_max)
			{
				const auto ascii = static_cast<std::uint8_t>(scalar);
				ascii_bits[ascii / 64u] |= (std::uint64_t{ 1 } << (ascii % 64u));
			}
			else
			{
				const auto lead = static_cast<std::uint8_t>(details::utf8_char_view(ch).front());
				non_ascii_lead_bits[lead / 64u] |= (std::uint64_t{ 1 } << (lead % 64u));
				insert_non_ascii_scalar(scalar);
			}
		}
	}

	[[nodiscard]]
	constexpr bool has_ascii() const noexcept
	{
		return ascii_bits[0] != 0 || ascii_bits[1] != 0;
	}

	[[nodiscard]]
	constexpr bool has_non_ascii() const noexcept
	{
		return non_ascii_count != 0 || non_ascii_overflow;
	}

	[[nodiscard]]
	constexpr bool has_non_ascii_overflow() const noexcept
	{
		return non_ascii_overflow;
	}

	[[nodiscard]]
	constexpr bool matches_ascii(std::uint8_t ascii) const noexcept
	{
		return (ascii_bits[ascii / 64u] & (std::uint64_t{ 1 } << (ascii % 64u))) != 0;
	}

	[[nodiscard]]
	constexpr bool may_match_non_ascii_lead_byte(std::uint8_t lead) const noexcept
	{
		return (non_ascii_lead_bits[lead / 64u] & (std::uint64_t{ 1 } << (lead % 64u))) != 0;
	}

	[[nodiscard]]
	constexpr bool matches_non_ascii_scalar(std::uint32_t scalar) const noexcept
	{
		if (non_ascii_count == 0 && !non_ascii_overflow)
		{
			return false;
		}

		if (!non_ascii_overflow)
		{
			return contains_non_ascii_scalar(scalar);
		}

		for (utf8_char candidate : chars)
		{
			if (!candidate.is_ascii() && candidate.as_scalar() == scalar)
			{
				return true;
			}
		}

		return false;
	}

	[[nodiscard]]
	constexpr bool operator()(utf8_char ch) const noexcept
	{
		const auto scalar = ch.as_scalar();
		if (scalar <= encoding_constants::ascii_scalar_max)
		{
			return matches_ascii(static_cast<std::uint8_t>(scalar));
		}

		return matches_non_ascii_scalar(scalar);
	}

private:
	constexpr void insert_non_ascii_scalar(std::uint32_t scalar) noexcept
	{
		if (non_ascii_overflow)
		{
			return;
		}

		std::size_t first = 0;
		std::size_t last = non_ascii_count;
		while (first < last)
		{
			const auto middle = first + (last - first) / 2;
			if (non_ascii_scalars[middle] < scalar)
			{
				first = middle + 1;
			}
			else
			{
				last = middle;
			}
		}

		if (first != non_ascii_count && non_ascii_scalars[first] == scalar)
		{
			return;
		}

		if (non_ascii_count == non_ascii_inline_capacity)
		{
			non_ascii_overflow = true;
			non_ascii_count = 0;
			return;
		}

		for (std::size_t i = non_ascii_count; i != first; --i)
		{
			non_ascii_scalars[i] = non_ascii_scalars[i - 1];
		}

		non_ascii_scalars[first] = scalar;
		++non_ascii_count;
	}

	[[nodiscard]]
	constexpr bool contains_non_ascii_scalar(std::uint32_t scalar) const noexcept
	{
		std::size_t first = 0;
		std::size_t last = non_ascii_count;
		while (first < last)
		{
			const auto middle = first + (last - first) / 2;
			const auto candidate = non_ascii_scalars[middle];
			if (candidate == scalar)
			{
				return true;
			}

			if (candidate < scalar)
			{
				first = middle + 1;
			}
			else
			{
				last = middle;
			}
		}

		return false;
	}
};

struct utf8_predicate_match
{
	std::size_t pos = std::u8string_view::npos;
	std::uint8_t size = 0;
};

inline constexpr utf8_char utf8_char_from_bytes_at(
	std::u8string_view base,
	std::size_t pos) noexcept
{
	const auto size = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base[pos]));
	return utf8_char::from_utf8_bytes_unchecked(base.data() + pos, size);
}

inline constexpr std::size_t previous_utf8_scalar_boundary(
	std::u8string_view base,
	std::size_t pos) noexcept
{
	if (pos == 0)
	{
		return 0;
	}

	pos = (std::min)(pos, base.size());
	--pos;
	while (pos != 0
		&& !details::is_utf8_lead_byte(static_cast<std::uint8_t>(base[pos])))
	{
		--pos;
	}

	return pos;
}

inline constexpr utf8_predicate_match find_utf8_predicate_match(
	std::u8string_view base,
	std::size_t pos,
	const utf8_char_span_matcher& matcher) noexcept
{
	const auto has_ascii = matcher.has_ascii();
	const auto has_non_ascii = matcher.has_non_ascii();
	while (pos < base.size())
	{
		if (!has_ascii)
		{
			pos += details::ascii_prefix_length(base.substr(pos));
			if (pos == base.size())
			{
				return {};
			}
		}

		const auto lead = static_cast<std::uint8_t>(base[pos]);
		if (lead <= encoding_constants::ascii_scalar_max)
		{
			if (matcher.matches_ascii(lead))
			{
				return { pos, 1 };
			}

			++pos;
			continue;
		}

		const auto size = static_cast<std::uint8_t>(details::utf8_byte_count_from_lead(lead));
		if (has_non_ascii && matcher.may_match_non_ascii_lead_byte(lead))
		{
			const auto scalar = details::decode_valid_utf8_char(base.data() + pos, size);
			if (matcher.matches_non_ascii_scalar(scalar))
			{
				return { pos, size };
			}
		}

		pos += size;
	}

	return {};
}

inline constexpr utf8_predicate_match rfind_utf8_predicate_match(
	std::u8string_view base,
	std::size_t end_exclusive,
	const utf8_char_span_matcher& matcher) noexcept
{
	if (base.empty() || end_exclusive == 0)
	{
		return {};
	}

	const auto has_non_ascii = matcher.has_non_ascii();
	for (std::size_t pos = details::previous_utf8_scalar_boundary(base, end_exclusive);; pos = details::previous_utf8_scalar_boundary(base, pos))
	{
		const auto lead = static_cast<std::uint8_t>(base[pos]);
		const auto size = static_cast<std::uint8_t>(lead <= encoding_constants::ascii_scalar_max
			? 1u
			: details::utf8_byte_count_from_lead(lead));
		if (lead <= encoding_constants::ascii_scalar_max)
		{
			if (matcher.matches_ascii(lead))
			{
				return { pos, size };
			}
		}
		else if (has_non_ascii && matcher.may_match_non_ascii_lead_byte(lead))
		{
			const auto scalar = details::decode_valid_utf8_char(base.data() + pos, size);
			if (matcher.matches_non_ascii_scalar(scalar))
			{
				return { pos, size };
			}
		}

		if (pos == 0)
		{
			return {};
		}
	}
}

inline constexpr utf8_predicate_match find_utf8_non_ascii_span_match(
	std::u8string_view base,
	std::size_t pos,
	const utf8_char_span_matcher& matcher) noexcept
{
	utf8_predicate_match result{};
	for (utf8_char ch : matcher.chars)
	{
		if (ch.is_ascii())
		{
			continue;
		}

		const auto needle = details::utf8_char_view(ch);
		const auto match = details::find_utf8_exact(base, needle, pos);
		if (match != std::u8string_view::npos
			&& (result.pos == std::u8string_view::npos || match < result.pos))
		{
			result = { match, static_cast<std::uint8_t>(needle.size()) };
			if (match == pos)
			{
				break;
			}
		}
	}

	return result;
}

inline constexpr utf8_predicate_match rfind_utf8_non_ascii_span_match(
	std::u8string_view base,
	std::size_t pos,
	const utf8_char_span_matcher& matcher) noexcept
{
	utf8_predicate_match result{};
	for (utf8_char ch : matcher.chars)
	{
		if (ch.is_ascii())
		{
			continue;
		}

		const auto needle = details::utf8_char_view(ch);
		if (needle.size() > base.size())
		{
			continue;
		}

		const auto max_start = pos == std::u8string_view::npos
			? base.size() - needle.size()
			: (std::min)(pos, base.size() - needle.size());
		const auto match = details::rfind_utf8_exact(base, needle, max_start);
		if (match != std::u8string_view::npos
			&& (result.pos == std::u8string_view::npos || match > result.pos))
		{
			result = { match, static_cast<std::uint8_t>(needle.size()) };
		}
	}

	return result;
}

template <utf8_char_predicate Pred>
inline constexpr utf8_predicate_match find_utf8_predicate_match(
	std::u8string_view base,
	std::size_t pos,
	const Pred& pred) noexcept
{
	while (pos < base.size())
	{
		const auto ch = details::utf8_char_from_bytes_at(base, pos);
		const auto size = static_cast<std::uint8_t>(ch.code_unit_count());
		if (std::invoke(pred, ch))
		{
			return { pos, size };
		}

		pos += size;
	}

	return {};
}

template <utf8_char_predicate Pred>
inline constexpr utf8_predicate_match rfind_utf8_predicate_match(
	std::u8string_view base,
	std::size_t end_exclusive,
	const Pred& pred) noexcept
{
	if (base.empty() || end_exclusive == 0)
	{
		return {};
	}

	for (std::size_t pos = details::previous_utf8_scalar_boundary(base, end_exclusive);; pos = details::previous_utf8_scalar_boundary(base, pos))
	{
		const auto ch = details::utf8_char_from_bytes_at(base, pos);
		const auto size = static_cast<std::uint8_t>(ch.code_unit_count());
		if (std::invoke(pred, ch))
		{
			return { pos, size };
		}

		if (pos == 0)
		{
			return {};
		}
	}
}

template <utf8_char_predicate Pred>
inline constexpr bool utf8_predicate_input_ends_with_delimiter(
	std::u8string_view base,
	const Pred& pred) noexcept
{
	if (base.empty())
	{
		return false;
	}

	return details::rfind_utf8_predicate_match(base, base.size(), pred).pos
		+ details::rfind_utf8_predicate_match(base, base.size(), pred).size
		== base.size();
}

template <typename View, bool DropTrailingEmpty, typename DelimiterStorage>
class basic_utf8_split_view
	: public std::ranges::view_interface<basic_utf8_split_view<View, DropTrailingEmpty, DelimiterStorage>>
{
public:
	static constexpr basic_utf8_split_view from_delimiter_storage(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
	{
		return basic_utf8_split_view{ base, delimiter_storage };
	}

	class iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			std::size_t current,
			std::size_t next_delimiter) noexcept
			: base_(base),
			  delimiter_(delimiter),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto segment_end = next_delimiter_ == std::u8string_view::npos
				? base_.size()
				: next_delimiter_;
			return View::from_bytes_unchecked(base_.substr(current_, segment_end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_ == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			const auto next_current = next_delimiter_ + delimiter_.size();
			if constexpr (DropTrailingEmpty)
			{
				if (next_current == base_.size())
				{
					current_ = std::u8string_view::npos;
					next_delimiter_ = std::u8string_view::npos;
					return *this;
				}
			}

			current_ = next_current;
			next_delimiter_ = details::find_utf8_split_delimiter(base_, delimiter_, current_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		constexpr iterator& operator--() noexcept
		{
			const auto old_current = current_;
			const auto current_for_search = [&]() constexpr noexcept {
				if constexpr (DropTrailingEmpty)
				{
					if (old_current == std::u8string_view::npos
						&& details::utf8_split_input_ends_with_delimiter(base_, delimiter_))
					{
						return base_.size();
					}
				}

				return old_current;
			}();
			current_ = basic_utf8_split_view::find_previous_segment_start(base_, delimiter_, current_for_search);
			next_delimiter_ = old_current == std::u8string_view::npos
				? details::find_utf8_split_delimiter(base_, delimiter_, current_)
				: old_current - delimiter_.size();
			return *this;
		}

		constexpr iterator operator--(int) noexcept
		{
			iterator old = *this;
			--(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
		{
			return lhs.base_.data() == rhs.base_.data()
				&& lhs.base_.size() == rhs.base_.size()
				&& lhs.delimiter_.data() == rhs.delimiter_.data()
				&& lhs.delimiter_.size() == rhs.delimiter_.size()
				&& lhs.current_ == rhs.current_
				&& lhs.next_delimiter_ == rhs.next_delimiter_;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		std::size_t current_ = 0;
		std::size_t next_delimiter_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		const auto delimiter = delimiter_view();
		return iterator{
			base_,
			delimiter,
			0,
			details::find_utf8_split_delimiter(base_, delimiter, 0)
		};
	}

	constexpr iterator end() const noexcept
	{
		const auto delimiter = delimiter_view();
		return iterator{
			base_,
			delimiter,
			std::u8string_view::npos,
			std::u8string_view::npos
		};
	}

private:
	constexpr explicit basic_utf8_split_view(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
		: base_(base), delimiter_storage_(delimiter_storage)
	{}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return delimiter_storage_.view();
	}

	static constexpr std::size_t find_previous_segment_start(
		std::u8string_view base,
		std::u8string_view delimiter,
		std::size_t current) noexcept
	{
		std::size_t previous_start = 0;
		std::size_t next = details::find_utf8_split_delimiter(base, delimiter, 0);
		while (next != std::u8string_view::npos
			&& next + delimiter.size() < current)
		{
			previous_start = next + delimiter.size();
			next = details::find_utf8_split_delimiter(base, delimiter, previous_start);
		}

		if (current == std::u8string_view::npos)
		{
			while (next != std::u8string_view::npos)
			{
				previous_start = next + delimiter.size();
				next = details::find_utf8_split_delimiter(base, delimiter, previous_start);
			}
		}

		return previous_start;
	}

	std::u8string_view base_{};
	DelimiterStorage delimiter_storage_{};
};

template <typename View, bool DropTrailingEmpty>
using utf8_split_view = basic_utf8_split_view<View, DropTrailingEmpty, borrowed_utf8_split_delimiter>;

template <typename View, bool DropTrailingEmpty>
using utf8_split_char_view = basic_utf8_split_view<View, DropTrailingEmpty, owned_utf8_split_char_delimiter>;

inline constexpr std::size_t find_utf8_non_delimiter_boundary(
	std::u8string_view base,
	std::u8string_view delimiter,
	std::size_t pos) noexcept
{
	if (pos >= base.size())
	{
		return std::u8string_view::npos;
	}

	if (delimiter.empty())
	{
		return pos;
	}

	while (pos < base.size()
		&& details::find_utf8_split_delimiter(base, delimiter, pos) == pos)
	{
		pos += delimiter.size();
	}

	return pos == base.size() ? std::u8string_view::npos : pos;
}

template <typename View, typename DelimiterStorage>
class basic_utf8_split_trimmed_view
	: public std::ranges::view_interface<basic_utf8_split_trimmed_view<View, DelimiterStorage>>
{
public:
	static constexpr basic_utf8_split_trimmed_view from_delimiter_storage(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
	{
		return basic_utf8_split_trimmed_view{ base, delimiter_storage };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			std::size_t current,
			std::size_t next_delimiter) noexcept
			: base_(base),
			  delimiter_(delimiter),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{
		}

		constexpr reference operator*() const noexcept
		{
			const auto end = next_delimiter_ == std::u8string_view::npos
				? base_.size()
				: next_delimiter_;
			return View::from_bytes_unchecked(base_.substr(current_, end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_ == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			current_ = details::find_utf8_non_delimiter_boundary(base_, delimiter_, next_delimiter_ + delimiter_.size());
			if (current_ == std::u8string_view::npos)
			{
				next_delimiter_ = std::u8string_view::npos;
				return *this;
			}

			next_delimiter_ = delimiter_.empty()
				? std::u8string_view::npos
				: details::find_utf8_split_delimiter(base_, delimiter_, current_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		std::size_t current_ = std::u8string_view::npos;
		std::size_t next_delimiter_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		const auto delimiter = delimiter_view();
		const auto current = details::find_utf8_non_delimiter_boundary(base_, delimiter, 0);
		if (current == std::u8string_view::npos)
		{
			return iterator{};
		}

		return iterator{
			base_,
			delimiter,
			current,
			delimiter.empty()
				? std::u8string_view::npos
				: details::find_utf8_split_delimiter(base_, delimiter, current)
		};
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_split_trimmed_view(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
		: base_(base), delimiter_storage_(delimiter_storage)
	{
	}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return delimiter_storage_.view();
	}

	std::u8string_view base_{};
	DelimiterStorage delimiter_storage_{};
};

template <typename View>
using utf8_split_trimmed_view = basic_utf8_split_trimmed_view<View, borrowed_utf8_split_delimiter>;

template <typename View>
using utf8_split_trimmed_char_view = basic_utf8_split_trimmed_view<View, owned_utf8_split_char_delimiter>;

template <typename View, bool Reverse, typename DelimiterStorage>
class basic_utf8_splitn_view
	: public std::ranges::view_interface<basic_utf8_splitn_view<View, Reverse, DelimiterStorage>>
{
public:
	static constexpr basic_utf8_splitn_view from_delimiter_storage(
		std::u8string_view base,
		DelimiterStorage delimiter_storage,
		std::size_t count) noexcept
	{
		return basic_utf8_splitn_view{ base, delimiter_storage, count };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			std::size_t current,
			std::size_t delimiter_pos,
			std::size_t remaining) noexcept
			: base_(base),
			  delimiter_(delimiter),
			  current_(current),
			  delimiter_pos_(delimiter_pos),
			  remaining_(remaining)
		{}

		constexpr reference operator*() const noexcept
		{
			if constexpr (Reverse)
			{
				if (remaining_ == 1 || delimiter_pos_ == std::u8string_view::npos)
				{
					return View::from_bytes_unchecked(base_.substr(0, current_));
				}

				const auto segment_start = delimiter_pos_ + delimiter_.size();
				return View::from_bytes_unchecked(base_.substr(segment_start, current_ - segment_start));
			}
			else
			{
				if (remaining_ == 1 || delimiter_pos_ == std::u8string_view::npos)
				{
					return View::from_bytes_unchecked(base_.substr(current_));
				}

				return View::from_bytes_unchecked(base_.substr(current_, delimiter_pos_ - current_));
			}
		}

		constexpr iterator& operator++() noexcept
		{
			if (remaining_ == 1 || delimiter_pos_ == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				delimiter_pos_ = std::u8string_view::npos;
				remaining_ = 0;
				return *this;
			}

			--remaining_;
			if constexpr (Reverse)
			{
				current_ = delimiter_pos_;
				delimiter_pos_ = details::rfind_utf8_split_delimiter(base_, delimiter_, current_);
			}
			else
			{
				current_ = delimiter_pos_ + delimiter_.size();
				delimiter_pos_ = details::find_utf8_split_delimiter(base_, delimiter_, current_);
			}
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.remaining_ == 0;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.remaining_ == 0;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		std::size_t current_ = std::u8string_view::npos;
		std::size_t delimiter_pos_ = std::u8string_view::npos;
		std::size_t remaining_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		if (count_ == 0)
		{
			return iterator{};
		}

		const auto delimiter = delimiter_view();
		if constexpr (Reverse)
		{
			return iterator{
				base_,
				delimiter,
				base_.size(),
				details::rfind_utf8_split_delimiter(base_, delimiter, base_.size()),
				count_
			};
		}
		else
		{
			return iterator{
				base_,
				delimiter,
				0,
				details::find_utf8_split_delimiter(base_, delimiter, 0),
				count_
			};
		}
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_splitn_view(
		std::u8string_view base,
		DelimiterStorage delimiter_storage,
		std::size_t count) noexcept
		: base_(base), delimiter_storage_(delimiter_storage), count_(count)
	{}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return delimiter_storage_.view();
	}

	std::u8string_view base_{};
	DelimiterStorage delimiter_storage_{};
	std::size_t count_ = 0;
};

template <typename View, bool Reverse>
using utf8_splitn_view = basic_utf8_splitn_view<View, Reverse, borrowed_utf8_split_delimiter>;

template <typename View, bool Reverse>
using utf8_splitn_char_view = basic_utf8_splitn_view<View, Reverse, owned_utf8_split_char_delimiter>;

template <typename Allocator>
inline constexpr std::basic_string<char8_t, std::char_traits<char8_t>, Allocator> replace_utf8_bytes_copy(
	std::u8string_view source,
	std::u8string_view needle,
	std::u8string_view replacement,
	std::size_t count,
	const Allocator& alloc)
{
	if (needle.empty() || count == 0)
	{
		return std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>{ source, alloc };
	}

	if consteval
	{
		std::size_t replacements = 0;
		for (std::size_t cursor = 0; replacements != count;)
		{
			const auto match = details::find_utf8_exact(source, needle, cursor);
			if (match == std::u8string_view::npos)
			{
				break;
			}

			cursor = match + needle.size();
			++replacements;
		}

		if (replacements == 0)
		{
			return std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>{ source, alloc };
		}

		std::size_t output_size = source.size();
		if (replacement.size() >= needle.size())
		{
			output_size += replacements * (replacement.size() - needle.size());
		}
		else
		{
			output_size -= replacements * (needle.size() - replacement.size());
		}

		std::basic_string<char8_t, std::char_traits<char8_t>, Allocator> result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t cursor = 0;
				std::size_t write_index = 0;
				std::size_t replacements_done = 0;
				while (replacements_done != replacements)
				{
					const auto match = details::find_utf8_exact(source, needle, cursor);
					const auto prefix_size = match - cursor;
					std::ranges::copy_n(source.data() + cursor, prefix_size, buffer + write_index);
					write_index += prefix_size;
					std::ranges::copy(replacement, buffer + write_index);
					write_index += replacement.size();
					cursor = match + needle.size();
					++replacements_done;
				}

				std::ranges::copy_n(source.data() + cursor, source.size() - cursor, buffer + write_index);
				return output_size;
			});

		return result;
	}

	const details::utf8_runtime_exact_searcher searcher{ needle };
	std::size_t replacements = 0;
	for (std::size_t cursor = 0; replacements != count;)
	{
		const auto match = searcher.find(source, cursor);
		if (match == std::u8string_view::npos)
		{
			break;
		}

		cursor = match + needle.size();
		++replacements;
	}

	if (replacements == 0)
	{
		return std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>{ source, alloc };
	}

	std::size_t output_size = source.size();
	if (replacement.size() >= needle.size())
	{
		output_size += replacements * (replacement.size() - needle.size());
	}
	else
	{
		output_size -= replacements * (needle.size() - replacement.size());
	}

	std::basic_string<char8_t, std::char_traits<char8_t>, Allocator> result{ alloc };
	result.resize_and_overwrite(output_size,
		[&](char8_t* buffer, std::size_t) noexcept
		{
			std::size_t cursor = 0;
			std::size_t write_index = 0;
			std::size_t replacements_done = 0;
			while (replacements_done != replacements)
			{
				const auto match = searcher.find(source, cursor);
				const auto prefix_size = match - cursor;
				std::ranges::copy_n(source.data() + cursor, prefix_size, buffer + write_index);
				write_index += prefix_size;
				std::ranges::copy(replacement, buffer + write_index);
				write_index += replacement.size();
				cursor = match + needle.size();
				++replacements_done;
			}

			std::ranges::copy_n(source.data() + cursor, source.size() - cursor, buffer + write_index);
			return output_size;
		});

	return result;
}

	template <typename Allocator, utf8_char_predicate Pred>
inline constexpr std::basic_string<char8_t, std::char_traits<char8_t>, Allocator> replace_utf8_chars_if_copy(
	std::u8string_view source,
	const Pred& pred,
	std::u8string_view replacement,
	std::size_t count,
	const Allocator& alloc)
{
	if (count == 0)
	{
		return std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>{ source, alloc };
	}

	std::size_t replacements = 0;
	std::size_t removed_size = 0;
	for (std::size_t pos = 0; pos < source.size();)
	{
		const auto ch = details::utf8_char_from_bytes_at(source, pos);
		const auto size = static_cast<std::size_t>(ch.code_unit_count());
		if (replacements != count && std::invoke(pred, ch))
		{
			removed_size += size;
			++replacements;
		}

		pos += size;
	}

	if (replacements == 0)
	{
		return std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>{ source, alloc };
	}

	const auto output_size = source.size() - removed_size + (replacements * replacement.size());
	std::basic_string<char8_t, std::char_traits<char8_t>, Allocator> result{ alloc };
	result.resize_and_overwrite(output_size,
		[&](char8_t* buffer, std::size_t) noexcept
		{
			std::size_t write_index = 0;
			std::size_t replacements_done = 0;
			for (std::size_t pos = 0; pos < source.size();)
			{
				const auto ch = details::utf8_char_from_bytes_at(source, pos);
				const auto size = static_cast<std::size_t>(ch.code_unit_count());
				if (replacements_done != replacements && std::invoke(pred, ch))
				{
					std::ranges::copy(replacement, buffer + write_index);
					write_index += replacement.size();
					++replacements_done;
				}
				else
				{
					std::ranges::copy_n(source.data() + pos, size, buffer + write_index);
					write_index += size;
				}

				pos += size;
			}

			return output_size;
		});

	return result;
}

template <typename View, typename DelimiterStorage>
class basic_utf8_split_inclusive_view
	: public std::ranges::view_interface<basic_utf8_split_inclusive_view<View, DelimiterStorage>>
{
private:
	static constexpr basic_utf8_split_inclusive_view from_delimiter_storage(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
	{
		return basic_utf8_split_inclusive_view{ base, delimiter_storage };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			std::size_t current,
			std::size_t next_delimiter) noexcept
			: base_(base),
			  delimiter_(delimiter),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{
		}

		constexpr reference operator*() const noexcept
		{
			const auto end = next_delimiter_ == std::u8string_view::npos
				? base_.size()
				: next_delimiter_ + delimiter_.size();
			return View::from_bytes_unchecked(base_.substr(current_, end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_ == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			current_ = next_delimiter_ + delimiter_.size();
			if (current_ == base_.size())
			{
				current_ = std::u8string_view::npos;
				next_delimiter_ = std::u8string_view::npos;
				return *this;
			}

			next_delimiter_ = details::find_utf8_split_delimiter(base_, delimiter_, current_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		std::size_t current_ = std::u8string_view::npos;
		std::size_t next_delimiter_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		return iterator{
			base_,
			delimiter_view(),
			0,
			details::find_utf8_split_delimiter(base_, delimiter_view(), 0)
		};
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_split_inclusive_view(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
		: base_(base), delimiter_storage_(delimiter_storage)
	{
	}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return delimiter_storage_.view();
	}

	std::u8string_view base_{};
	DelimiterStorage delimiter_storage_{};
};

template <typename View>
using utf8_split_inclusive_view = basic_utf8_split_inclusive_view<View, borrowed_utf8_split_delimiter>;

template <typename View>
using utf8_split_inclusive_char_view = basic_utf8_split_inclusive_view<View, owned_utf8_split_char_delimiter>;

template <typename View, bool Reverse, typename DelimiterStorage>
class basic_utf8_match_indices_view
	: public std::ranges::view_interface<basic_utf8_match_indices_view<View, Reverse, DelimiterStorage>>
{
private:
	static constexpr basic_utf8_match_indices_view from_delimiter_storage(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
	{
		return basic_utf8_match_indices_view{ base, delimiter_storage };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, View>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			std::size_t current) noexcept
			: base_(base), delimiter_(delimiter), current_(current)
		{
		}

		constexpr reference operator*() const noexcept
		{
			return {
				current_,
				View::from_bytes_unchecked(base_.substr(current_, delimiter_.size()))
			};
		}

		constexpr iterator& operator++() noexcept
		{
			if constexpr (Reverse)
			{
				current_ = details::rfind_utf8_split_delimiter(base_, delimiter_, current_);
			}
			else
			{
				current_ = details::find_utf8_split_delimiter(base_, delimiter_, current_ + delimiter_.size());
			}

			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		std::size_t current_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		const auto delimiter = delimiter_view();
		if (delimiter.empty())
		{
			return iterator{};
		}

		if constexpr (Reverse)
		{
			return iterator{
				base_,
				delimiter,
				details::rfind_utf8_split_delimiter(base_, delimiter, base_.size())
			};
		}
		else
		{
			return iterator{
				base_,
				delimiter,
				details::find_utf8_split_delimiter(base_, delimiter, 0)
			};
		}
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_match_indices_view(
		std::u8string_view base,
		DelimiterStorage delimiter_storage) noexcept
		: base_(base), delimiter_storage_(delimiter_storage)
	{
	}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return delimiter_storage_.view();
	}

	std::u8string_view base_{};
	DelimiterStorage delimiter_storage_{};
};

template <typename View, bool Reverse>
using utf8_match_indices_view = basic_utf8_match_indices_view<View, Reverse, borrowed_utf8_split_delimiter>;

template <typename View, bool Reverse>
using utf8_match_indices_char_view = basic_utf8_match_indices_view<View, Reverse, owned_utf8_split_char_delimiter>;

template <typename View, bool DropTrailingEmpty, utf8_char_predicate Pred>
class basic_utf8_predicate_split_view
	: public std::ranges::view_interface<basic_utf8_predicate_split_view<View, DropTrailingEmpty, Pred>>
{
private:
	static constexpr basic_utf8_predicate_split_view from_predicate(
		std::u8string_view base,
		Pred pred) noexcept
	{
		return basic_utf8_predicate_split_view{ base, pred };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			const Pred* pred,
			std::size_t current,
			utf8_predicate_match next_delimiter) noexcept
			: base_(base),
			  pred_(pred),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{
		}

		constexpr reference operator*() const noexcept
		{
			const auto segment_end = next_delimiter_.pos == std::u8string_view::npos
				? base_.size()
				: next_delimiter_.pos;
			return View::from_bytes_unchecked(base_.substr(current_, segment_end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_.pos == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			const auto next_current = next_delimiter_.pos + next_delimiter_.size;
			if constexpr (DropTrailingEmpty)
			{
				if (next_current == base_.size())
				{
					current_ = std::u8string_view::npos;
					next_delimiter_ = {};
					return *this;
				}
			}

			current_ = next_current;
			next_delimiter_ = details::find_utf8_predicate_match(base_, current_, *pred_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		constexpr iterator& operator--() noexcept
		{
			const auto current_for_search = [&]() constexpr noexcept {
				if constexpr (DropTrailingEmpty)
				{
					if (current_ == std::u8string_view::npos)
					{
						const auto trailing = details::rfind_utf8_predicate_match(base_, base_.size(), *pred_);
						if (trailing.pos != std::u8string_view::npos
							&& trailing.pos + trailing.size == base_.size())
						{
							return base_.size();
						}
					}
				}

				return current_;
			}();
			current_ = basic_utf8_predicate_split_view::find_previous_segment_start(base_, *pred_, current_for_search);
			next_delimiter_ = details::find_utf8_predicate_match(base_, current_, *pred_);
			return *this;
		}

		constexpr iterator operator--(int) noexcept
		{
			iterator old = *this;
			--(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
		{
			return lhs.base_.data() == rhs.base_.data()
				&& lhs.base_.size() == rhs.base_.size()
				&& lhs.pred_ == rhs.pred_
				&& lhs.current_ == rhs.current_
				&& lhs.next_delimiter_.pos == rhs.next_delimiter_.pos
				&& lhs.next_delimiter_.size == rhs.next_delimiter_.size;
		}

	private:
		std::u8string_view base_{};
		const Pred* pred_ = nullptr;
		std::size_t current_ = 0;
		utf8_predicate_match next_delimiter_{};
	};

	constexpr iterator begin() const noexcept
	{
		return iterator{
			base_,
			&pred_,
			0,
			details::find_utf8_predicate_match(base_, 0, pred_)
		};
	}

	constexpr iterator end() const noexcept
	{
		return iterator{
			base_,
			&pred_,
			std::u8string_view::npos,
			{}
		};
	}

private:
	constexpr explicit basic_utf8_predicate_split_view(
		std::u8string_view base,
		Pred pred) noexcept
		: base_(base), pred_(pred)
	{
	}

	static constexpr std::size_t find_previous_segment_start(
		std::u8string_view base,
		const Pred& pred,
		std::size_t current) noexcept
	{
		std::size_t previous_start = 0;
		for (auto next = details::find_utf8_predicate_match(base, 0, pred);
			next.pos != std::u8string_view::npos && next.pos + next.size < current;
			next = details::find_utf8_predicate_match(base, previous_start, pred))
		{
			previous_start = next.pos + next.size;
		}

		if (current == std::u8string_view::npos)
		{
			for (auto next = details::find_utf8_predicate_match(base, 0, pred);
				next.pos != std::u8string_view::npos;
				next = details::find_utf8_predicate_match(base, previous_start, pred))
			{
				previous_start = next.pos + next.size;
			}
		}

		return previous_start;
	}

	std::u8string_view base_{};
	UTF8_RANGES_NO_UNIQUE_ADDRESS Pred pred_;
};

template <utf8_char_predicate Pred>
inline constexpr std::size_t find_utf8_non_predicate_boundary(
	std::u8string_view base,
	std::size_t pos,
	const Pred& pred) noexcept
{
	while (pos < base.size())
	{
		const auto ch = details::utf8_char_from_bytes_at(base, pos);
		if (!std::invoke(pred, ch))
		{
			return pos;
		}

		pos += ch.code_unit_count();
	}

	return std::u8string_view::npos;
}

template <typename View, utf8_char_predicate Pred>
class basic_utf8_predicate_split_trimmed_view
	: public std::ranges::view_interface<basic_utf8_predicate_split_trimmed_view<View, Pred>>
{
private:
	static constexpr basic_utf8_predicate_split_trimmed_view from_predicate(
		std::u8string_view base,
		Pred pred) noexcept
	{
		return basic_utf8_predicate_split_trimmed_view{ base, pred };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			const Pred* pred,
			std::size_t current,
			utf8_predicate_match next_delimiter) noexcept
			: base_(base),
			  pred_(pred),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{
		}

		constexpr reference operator*() const noexcept
		{
			const auto end = next_delimiter_.pos == std::u8string_view::npos
				? base_.size()
				: next_delimiter_.pos;
			return View::from_bytes_unchecked(base_.substr(current_, end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_.pos == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			current_ = details::find_utf8_non_predicate_boundary(base_, next_delimiter_.pos + next_delimiter_.size, *pred_);
			if (current_ == std::u8string_view::npos)
			{
				next_delimiter_ = {};
				return *this;
			}

			next_delimiter_ = details::find_utf8_predicate_match(base_, current_, *pred_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		const Pred* pred_ = nullptr;
		std::size_t current_ = std::u8string_view::npos;
		utf8_predicate_match next_delimiter_{};
	};

	constexpr iterator begin() const noexcept
	{
		const auto current = details::find_utf8_non_predicate_boundary(base_, 0, pred_);
		if (current == std::u8string_view::npos)
		{
			return iterator{};
		}

		return iterator{
			base_,
			&pred_,
			current,
			details::find_utf8_predicate_match(base_, current, pred_)
		};
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_predicate_split_trimmed_view(
		std::u8string_view base,
		Pred pred) noexcept
		: base_(base), pred_(pred)
	{
	}

	std::u8string_view base_{};
	UTF8_RANGES_NO_UNIQUE_ADDRESS Pred pred_;
};

template <typename View, bool Reverse, utf8_char_predicate Pred>
class basic_utf8_predicate_splitn_view
	: public std::ranges::view_interface<basic_utf8_predicate_splitn_view<View, Reverse, Pred>>
{
private:
	static constexpr basic_utf8_predicate_splitn_view from_predicate(
		std::u8string_view base,
		Pred pred,
		std::size_t count) noexcept
	{
		return basic_utf8_predicate_splitn_view{ base, pred, count };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			const Pred* pred,
			std::size_t current,
			utf8_predicate_match delimiter,
			std::size_t remaining) noexcept
			: base_(base),
			  pred_(pred),
			  current_(current),
			  delimiter_(delimiter),
			  remaining_(remaining)
		{
		}

		constexpr reference operator*() const noexcept
		{
			if constexpr (Reverse)
			{
				if (remaining_ == 1 || delimiter_.pos == std::u8string_view::npos)
				{
					return View::from_bytes_unchecked(base_.substr(0, current_));
				}

				const auto segment_start = delimiter_.pos + delimiter_.size;
				return View::from_bytes_unchecked(base_.substr(segment_start, current_ - segment_start));
			}
			else
			{
				if (remaining_ == 1 || delimiter_.pos == std::u8string_view::npos)
				{
					return View::from_bytes_unchecked(base_.substr(current_));
				}

				return View::from_bytes_unchecked(base_.substr(current_, delimiter_.pos - current_));
			}
		}

		constexpr iterator& operator++() noexcept
		{
			if (remaining_ == 1 || delimiter_.pos == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				delimiter_ = {};
				remaining_ = 0;
				return *this;
			}

			--remaining_;
			if constexpr (Reverse)
			{
				current_ = delimiter_.pos;
				delimiter_ = details::rfind_utf8_predicate_match(base_, current_, *pred_);
			}
			else
			{
				current_ = delimiter_.pos + delimiter_.size;
				delimiter_ = details::find_utf8_predicate_match(base_, current_, *pred_);
			}
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.remaining_ == 0;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.remaining_ == 0;
		}

	private:
		std::u8string_view base_{};
		const Pred* pred_ = nullptr;
		std::size_t current_ = std::u8string_view::npos;
		utf8_predicate_match delimiter_{};
		std::size_t remaining_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		if (count_ == 0)
		{
			return iterator{};
		}

		if constexpr (Reverse)
		{
			return iterator{
				base_,
				&pred_,
				base_.size(),
				details::rfind_utf8_predicate_match(base_, base_.size(), pred_),
				count_
			};
		}
		else
		{
			return iterator{
				base_,
				&pred_,
				0,
				details::find_utf8_predicate_match(base_, 0, pred_),
				count_
			};
		}
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_predicate_splitn_view(
		std::u8string_view base,
		Pred pred,
		std::size_t count) noexcept
		: base_(base), pred_(pred), count_(count)
	{
	}

	std::u8string_view base_{};
	UTF8_RANGES_NO_UNIQUE_ADDRESS Pred pred_;
	std::size_t count_ = 0;
};

template <typename View, utf8_char_predicate Pred>
class basic_utf8_predicate_split_inclusive_view
	: public std::ranges::view_interface<basic_utf8_predicate_split_inclusive_view<View, Pred>>
{
private:
	static constexpr basic_utf8_predicate_split_inclusive_view from_predicate(
		std::u8string_view base,
		Pred pred) noexcept
	{
		return basic_utf8_predicate_split_inclusive_view{ base, pred };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			const Pred* pred,
			std::size_t current,
			utf8_predicate_match next_delimiter) noexcept
			: base_(base),
			  pred_(pred),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{
		}

		constexpr reference operator*() const noexcept
		{
			const auto end = next_delimiter_.pos == std::u8string_view::npos
				? base_.size()
				: next_delimiter_.pos + next_delimiter_.size;
			return View::from_bytes_unchecked(base_.substr(current_, end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_.pos == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			current_ = next_delimiter_.pos + next_delimiter_.size;
			if (current_ == base_.size())
			{
				current_ = std::u8string_view::npos;
				next_delimiter_ = {};
				return *this;
			}

			next_delimiter_ = details::find_utf8_predicate_match(base_, current_, *pred_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		const Pred* pred_ = nullptr;
		std::size_t current_ = std::u8string_view::npos;
		utf8_predicate_match next_delimiter_{};
	};

	constexpr iterator begin() const noexcept
	{
		return iterator{
			base_,
			&pred_,
			0,
			details::find_utf8_predicate_match(base_, 0, pred_)
		};
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_predicate_split_inclusive_view(
		std::u8string_view base,
		Pred pred) noexcept
		: base_(base), pred_(pred)
	{
	}

	std::u8string_view base_{};
	UTF8_RANGES_NO_UNIQUE_ADDRESS Pred pred_;
};

template <typename View, bool Reverse, utf8_char_predicate Pred>
class basic_utf8_predicate_match_indices_view
	: public std::ranges::view_interface<basic_utf8_predicate_match_indices_view<View, Reverse, Pred>>
{
private:
	static constexpr basic_utf8_predicate_match_indices_view from_predicate(
		std::u8string_view base,
		Pred pred) noexcept
	{
		return basic_utf8_predicate_match_indices_view{ base, pred };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:
	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, View>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			const Pred* pred,
			utf8_predicate_match current) noexcept
			: base_(base), pred_(pred), current_(current)
		{
		}

		constexpr reference operator*() const noexcept
		{
			return {
				current_.pos,
				View::from_bytes_unchecked(base_.substr(current_.pos, current_.size))
			};
		}

		constexpr iterator& operator++() noexcept
		{
			if constexpr (Reverse)
			{
				current_ = details::rfind_utf8_predicate_match(base_, current_.pos, *pred_);
			}
			else
			{
				current_ = details::find_utf8_predicate_match(base_, current_.pos + current_.size, *pred_);
			}

			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_.pos == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_.pos == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		const Pred* pred_ = nullptr;
		utf8_predicate_match current_{};
	};

	constexpr iterator begin() const noexcept
	{
		if constexpr (Reverse)
		{
			return iterator{
				base_,
				&pred_,
				details::rfind_utf8_predicate_match(base_, base_.size(), pred_)
			};
		}
		else
		{
			return iterator{
				base_,
				&pred_,
				details::find_utf8_predicate_match(base_, 0, pred_)
			};
		}
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit basic_utf8_predicate_match_indices_view(
		std::u8string_view base,
		Pred pred) noexcept
		: base_(base), pred_(pred)
	{
	}

	std::u8string_view base_{};
	UTF8_RANGES_NO_UNIQUE_ADDRESS Pred pred_;
};

inline constexpr bool utf8_char_is_whitespace_at(
	std::u8string_view base,
	std::size_t pos,
	bool ascii_only) noexcept
{
	const auto len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base[pos]));
	const auto ch = utf8_char::from_utf8_bytes_unchecked(base.data() + pos, len);
	return ascii_only ? ch.is_ascii_whitespace() : ch.is_whitespace();
}

inline constexpr std::size_t next_utf8_scalar_boundary(
	std::u8string_view base,
	std::size_t pos) noexcept
{
	return pos + details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base[pos]));
}

inline constexpr std::size_t find_utf8_non_whitespace_boundary(
	std::u8string_view base,
	std::size_t pos,
	bool ascii_only) noexcept
{
	while (pos < base.size())
	{
		if (!details::utf8_char_is_whitespace_at(base, pos, ascii_only))
		{
			return pos;
		}

		pos = details::next_utf8_scalar_boundary(base, pos);
	}

	return std::u8string_view::npos;
}

inline constexpr std::size_t find_utf8_whitespace_boundary(
	std::u8string_view base,
	std::size_t pos,
	bool ascii_only) noexcept
{
	while (pos < base.size())
	{
		if (details::utf8_char_is_whitespace_at(base, pos, ascii_only))
		{
			return pos;
		}

		pos = details::next_utf8_scalar_boundary(base, pos);
	}

	return std::u8string_view::npos;
}

inline constexpr std::size_t utf8_trim_end_boundary(
	std::u8string_view base,
	bool ascii_only) noexcept
{
	std::size_t end = 0;
	for (std::size_t pos = 0; pos < base.size();)
	{
		const auto next = details::next_utf8_scalar_boundary(base, pos);
		if (!details::utf8_char_is_whitespace_at(base, pos, ascii_only))
		{
			end = next;
		}

		pos = next;
	}

	return end;
}

template <typename View, bool AsciiOnly>
class utf8_whitespace_split_view : public std::ranges::view_interface<utf8_whitespace_split_view<View, AsciiOnly>>
{
private:
	static constexpr utf8_whitespace_split_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(base).has_value());
		return utf8_whitespace_split_view{ base };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::size_t current,
			std::size_t next_whitespace) noexcept
			: base_(base), current_(current), next_whitespace_(next_whitespace)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto end = next_whitespace_ == std::u8string_view::npos
				? base_.size()
				: next_whitespace_;
			return View::from_bytes_unchecked(base_.substr(current_, end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			current_ = details::find_utf8_non_whitespace_boundary(
				base_,
				next_whitespace_ == std::u8string_view::npos ? base_.size() : next_whitespace_,
				AsciiOnly);
			if (current_ == std::u8string_view::npos)
			{
				next_whitespace_ = std::u8string_view::npos;
				return *this;
			}

			next_whitespace_ = details::find_utf8_whitespace_boundary(base_, current_, AsciiOnly);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		std::size_t current_ = std::u8string_view::npos;
		std::size_t next_whitespace_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		const auto current = details::find_utf8_non_whitespace_boundary(base_, 0, AsciiOnly);
		if (current == std::u8string_view::npos)
		{
			return iterator{};
		}

		return iterator{
			base_,
			current,
			details::find_utf8_whitespace_boundary(base_, current, AsciiOnly)
		};
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit utf8_whitespace_split_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

template <typename Derived, typename View>
class utf8_string_crtp
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	[[nodiscard]]
	constexpr auto chars() const& noexcept
	{
		return views::utf8_view::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr auto chars() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		return views::owning_chars_view<Derived>{ std::move(static_cast<Derived&>(*this)) };
	}

	[[nodiscard]]
	constexpr auto reversed_chars() const& noexcept
	{
		return views::reversed_utf8_view::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr auto reversed_chars() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		return views::owning_reversed_chars_view<Derived>{ std::move(static_cast<Derived&>(*this)) };
	}

	[[nodiscard]]
	constexpr auto graphemes() const& noexcept -> views::grapheme_cluster_view<char8_t>;
	[[nodiscard]]
	constexpr auto graphemes() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		-> views::owning_grapheme_cluster_view<Derived>
		requires (!std::same_as<Derived, View>);
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_utf8_owned(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_ascii_lowercase(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_ascii_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_ascii_uppercase(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_ascii_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_lowercase(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc = Allocator()) const;
#if UTF8_RANGES_HAS_ICU
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	basic_utf8_string<Allocator> to_lowercase(locale_id locale, const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	basic_utf8_string<Allocator> to_lowercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	basic_utf8_string<Allocator> to_uppercase(locale_id locale, const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	basic_utf8_string<Allocator> to_uppercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	basic_utf8_string<Allocator> to_titlecase(locale_id locale, const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	basic_utf8_string<Allocator> case_fold(locale_id locale, const Allocator& alloc = Allocator()) const;
	[[nodiscard]]
	bool eq_ignore_case(View sv, locale_id locale) const;
	[[nodiscard]]
	bool starts_with_ignore_case(View sv, locale_id locale) const;
	[[nodiscard]]
	bool ends_with_ignore_case(View sv, locale_id locale) const;
	[[nodiscard]]
	std::weak_ordering compare_ignore_case(View sv, locale_id locale) const;
#endif
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_uppercase(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> normalize(
		normalization_form form,
		const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_nfc(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_nfd(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_nfkc(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> to_nfkd(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char8_t>>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> case_fold(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char16_t>>
	[[nodiscard]]
	constexpr basic_utf16_string<Allocator> to_utf16(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char32_t>>
	[[nodiscard]]
	constexpr basic_utf32_string<Allocator> to_utf32(const Allocator& alloc = Allocator()) const;
	[[nodiscard]]
	constexpr bool eq_ignore_case(View sv) const noexcept;
	[[nodiscard]]
	constexpr bool starts_with_ignore_case(View sv) const noexcept;
	[[nodiscard]]
	constexpr bool ends_with_ignore_case(View sv) const noexcept;
	[[nodiscard]]
	constexpr std::weak_ordering compare_ignore_case(View sv) const noexcept;

	[[nodiscard]]
	constexpr size_type size() const noexcept
	{
		return byte_view().size();
	}

	[[nodiscard]]
	constexpr bool empty() const noexcept
	{
		return byte_view().empty();
	}

	[[nodiscard]]
	constexpr bool is_ascii() const noexcept
	{
		return std::ranges::all_of(byte_view(),
			[](char8_t byte) noexcept
			{
				return static_cast<std::uint8_t>(byte) <= details::encoding_constants::ascii_scalar_max;
			});
	}

	[[nodiscard]]
	constexpr auto char_indices() const& noexcept
	{
		return utf8_char_indices_view::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr auto char_indices() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		return views::owning_char_indices_view<Derived>{ std::move(static_cast<Derived&>(*this)) };
	}

	[[nodiscard]]
	constexpr auto grapheme_indices() const& noexcept
	{
		return utf8_grapheme_indices_view<View>::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr auto grapheme_indices() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		return views::owning_grapheme_indices_view<Derived>{ std::move(static_cast<Derived&>(*this)) };
	}

	[[nodiscard]]
	constexpr bool is_grapheme_boundary(size_type index) const noexcept
	{
		return details::is_grapheme_boundary(byte_view(), index);
	}

	[[nodiscard]]
	constexpr bool is_normalized(normalization_form form) const
	{
		return normalize(form) == View::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr bool is_nfc() const
	{
		return is_normalized(normalization_form::nfc);
	}

	[[nodiscard]]
	constexpr bool is_nfd() const
	{
		return is_normalized(normalization_form::nfd);
	}

	[[nodiscard]]
	constexpr bool is_nfkc() const
	{
		return is_normalized(normalization_form::nfkc);
	}

	[[nodiscard]]
	constexpr bool is_nfkd() const
	{
		return is_normalized(normalization_form::nfkd);
	}

	[[nodiscard]]
	constexpr bool contains(utf8_char ch) const noexcept
	{
		return find(ch) != npos;
	}

	[[nodiscard]]
	constexpr bool contains(View sv) const noexcept
	{
		return find(sv) != npos;
	}

	[[nodiscard]]
	constexpr bool contains(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return false;
		}

		if (chars.size() == 1)
		{
			return contains(chars.front());
		}

		return find(details::utf8_char_span_matcher{ chars }) != npos;
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr bool contains(Pred pred) const noexcept
	{
		return find(pred) != npos;
	}

	[[nodiscard]]
	constexpr bool contains_grapheme(utf8_char ch) const noexcept
	{
		return find_grapheme(ch) != npos;
	}

	[[nodiscard]]
	constexpr bool contains_grapheme(View sv) const noexcept
	{
		return find_grapheme(sv) != npos;
	}

	[[nodiscard]]
	constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		if consteval
		{
			for (size_type index = pos; index != size(); ++index)
			{
				if (byte_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(ch, pos);
		}
	}

	[[nodiscard]]
	constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		std::array<char8_t, 4> bytes{};
		const auto needle_size = ch.encode_utf8<char8_t>(bytes.begin());
		return details::find_utf8_exact(byte_view(), std::u8string_view{ bytes.data(), needle_size }, pos);
	}

	[[nodiscard]]
	constexpr size_type find(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		const auto needle = sv.base();
		return details::find_utf8_exact(byte_view(), needle, pos);
	}

	[[nodiscard]]
	constexpr size_type find(std::span<const utf8_char> chars, size_type pos = 0) const noexcept
	{
		if (chars.empty())
		{
			return npos;
		}

		if (chars.size() == 1)
		{
			return find(chars.front(), pos);
		}

		pos = ceil_char_boundary((std::min)(size(), pos));
		const details::utf8_char_span_matcher matcher{ chars };
		if (!matcher.has_ascii() && !matcher.has_non_ascii_overflow())
		{
			return details::find_utf8_non_ascii_span_match(byte_view(), pos, matcher).pos;
		}

		return details::find_utf8_predicate_match(byte_view(), pos, matcher).pos;
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr size_type find(Pred pred, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		return details::find_utf8_predicate_match(byte_view(), pos, pred).pos;
	}

	[[nodiscard]]
	constexpr size_type find_grapheme(utf8_char ch, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(byte_view(), details::utf8_char_view(ch), pos);
	}

	[[nodiscard]]
	constexpr size_type find_grapheme(View sv, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(byte_view(), sv.base(), pos);
	}

	[[nodiscard]]
	constexpr size_type find_first_of(char8_t ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	[[nodiscard]]
	constexpr size_type find_first_of(utf8_char ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	[[nodiscard]]
	constexpr size_type find_first_of(View sv, size_type pos = 0) const noexcept
	{
		if (sv.empty())
		{
			return npos;
		}

		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && sv.contains(current);
		});

		return it == indices.end() ? npos : (*it).first;
	}

	[[nodiscard]]
	constexpr size_type find_first_not_of(char8_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		for (size_type index = pos; index != size(); ++index)
		{
			if (byte_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	[[nodiscard]]
	constexpr size_type find_first_not_of(utf8_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && current != ch;
		});

		return it == indices.end() ? npos : (*it).first;
	}

	[[nodiscard]]
	constexpr size_type find_first_not_of(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && !sv.contains(current);
		});

		return it == indices.end() ? npos : (*it).first;
	}

	[[nodiscard]]
	constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				if (byte_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(ch, pos);
		}
	}

	[[nodiscard]]
	constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept
	{
		std::array<char8_t, 4> bytes{};
		const auto needle_size = ch.encode_utf8<char8_t>(bytes.begin());
		if (needle_size > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		pos = floor_char_boundary((std::min)(pos, size() - needle_size));
		return details::rfind_utf8_exact(byte_view(), std::u8string_view{ bytes.data(), needle_size }, pos);
	}

	[[nodiscard]]
	constexpr size_type rfind(View sv, size_type pos = npos) const noexcept
	{
		const auto needle = sv.base();
		pos = floor_char_boundary((std::min)(size(), pos));
		if (needle.size() > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(pos, size() - needle.size()));
		return details::rfind_utf8_exact(byte_view(), needle, pos);
	}

	[[nodiscard]]
	constexpr size_type rfind(std::span<const utf8_char> chars, size_type pos = npos) const noexcept
	{
		if (chars.empty())
		{
			return npos;
		}

		if (chars.size() == 1)
		{
			return rfind(chars.front(), pos);
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		const details::utf8_char_span_matcher matcher{ chars };
		if (!matcher.has_ascii() && !matcher.has_non_ascii_overflow())
		{
			return details::rfind_utf8_non_ascii_span_match(byte_view(), pos, matcher).pos;
		}

		const auto end_exclusive = pos == npos
			? size()
			: pos == size()
				? size()
				: pos + details::utf8_char_from_bytes_at(byte_view(), pos).code_unit_count();
		return details::rfind_utf8_predicate_match(byte_view(), end_exclusive, matcher).pos;
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr size_type rfind(Pred pred, size_type pos = npos) const noexcept
	{
		pos = floor_char_boundary((std::min)(size(), pos));
		const auto end_exclusive = pos == npos
			? size()
			: pos == size()
				? size()
				: pos + details::utf8_char_from_bytes_at(byte_view(), pos).code_unit_count();
		return details::rfind_utf8_predicate_match(byte_view(), end_exclusive, pred).pos;
	}

	[[nodiscard]]
	constexpr size_type rfind_grapheme(utf8_char ch, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(byte_view(), details::utf8_char_view(ch), pos);
	}

	[[nodiscard]]
	constexpr size_type rfind_grapheme(View sv, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(byte_view(), sv.base(), pos);
	}

	[[nodiscard]]
	constexpr size_type find_last_of(char8_t ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	[[nodiscard]]
	constexpr size_type find_last_of(utf8_char ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	[[nodiscard]]
	constexpr size_type find_last_of(View sv, size_type pos = npos) const noexcept
	{
		if (empty() || sv.empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		if (pos == size())
		{
			pos = floor_char_boundary(size() - 1);
		}
		for (size_type index = pos;;)
		{
			if (sv.contains(char_at_unchecked(index)))
			{
				return index;
			}

			if (index == 0)
			{
				return npos;
			}

			index = floor_char_boundary(index - 1);
		}
	}

	[[nodiscard]]
	constexpr size_type find_last_not_of(char8_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		for (size_type index = pos + 1; index != 0;)
		{
			--index;
			if (byte_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	[[nodiscard]]
	constexpr size_type find_last_not_of(utf8_char ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		size_type result = npos;
		for (const auto [index, current] : char_indices())
		{
			if (index > pos)
			{
				break;
			}

			if (current != ch)
			{
				result = index;
			}
		}

		return result;
	}

	[[nodiscard]]
	constexpr size_type find_last_not_of(View sv, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		if (pos == size())
		{
			pos = floor_char_boundary(size() - 1);
		}
		for (size_type index = pos;;)
		{
			if (!sv.contains(char_at_unchecked(index)))
			{
				return index;
			}

			if (index == 0)
			{
				return npos;
			}

			index = floor_char_boundary(index - 1);
		}
	}

	[[nodiscard]]
	constexpr bool is_char_boundary(size_type index) const noexcept
	{
		if (index > size()) [[unlikely]]
		{
			return false;
		}

		if (index == 0 || index == size()) [[unlikely]]
		{
			return true;
		}

		return details::is_utf8_lead_byte(static_cast<std::uint8_t>(byte_view()[index]));
	}

	[[nodiscard]]
	constexpr size_type char_count() const noexcept
	{
		if (!std::is_constant_evaluated())
		{
			if (auto count = details::simdutf_count_utf8_if_available(byte_view()))
			{
				return static_cast<size_type>(*count);
			}
		}

		return static_cast<size_type>(std::ranges::distance(chars()));
	}

	[[nodiscard]]
	constexpr size_type grapheme_count() const noexcept
	{
		return details::grapheme_count(byte_view());
	}

	[[nodiscard]]
	constexpr auto split(utf8_char ch) const& noexcept
	{
		return utf8_split_char_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch });
	}

	[[nodiscard]]
	constexpr auto split(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	[[nodiscard]]
	constexpr auto split(View sv) const& noexcept
	{
		return utf8_split_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() });
	}

	[[nodiscard]]
	constexpr auto split(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_split_view<View, false, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred));
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	[[nodiscard]]
	constexpr auto split_trimmed(utf8_char ch) const& noexcept
	{
		return details::utf8_split_trimmed_char_view<View>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch });
	}

	[[nodiscard]]
	constexpr auto split_trimmed(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_trimmed_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	[[nodiscard]]
	constexpr auto split_trimmed(View sv) const& noexcept
	{
		return details::utf8_split_trimmed_view<View>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() });
	}

	[[nodiscard]]
	constexpr auto split_trimmed(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_trimmed_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split_trimmed(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_split_trimmed_view<View, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred));
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split_trimmed(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_trimmed_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	[[nodiscard]]
	constexpr auto split_whitespace() const& noexcept
	{
		return utf8_whitespace_split_view<View, false>::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr auto split_whitespace() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		return details::owning_string_view<Derived, details::split_whitespace_view_accessor>{
			std::move(static_cast<Derived&>(*this))
		};
	}

	[[nodiscard]]
	constexpr auto split_ascii_whitespace() const& noexcept
	{
		return utf8_whitespace_split_view<View, true>::from_bytes_unchecked(byte_view());
	}

	[[nodiscard]]
	constexpr auto split_ascii_whitespace() && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		return details::owning_string_view<Derived, details::split_ascii_whitespace_view_accessor>{
			std::move(static_cast<Derived&>(*this))
		};
	}

	[[nodiscard]]
	constexpr auto rsplit(utf8_char ch) const& noexcept
	{
		return std::views::reverse(split(ch));
	}

	[[nodiscard]]
	constexpr auto rsplit(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplit_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	[[nodiscard]]
	constexpr auto rsplit(View sv) const& noexcept
	{
		return std::views::reverse(split(sv));
	}

	[[nodiscard]]
	constexpr auto rsplit(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplit_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto rsplit(Pred pred) const& noexcept
	{
		return std::views::reverse(split(std::move(pred)));
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto rsplit(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplit_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	[[nodiscard]]
	constexpr auto split_terminator(utf8_char ch) const& noexcept
	{
		return utf8_split_char_view<View, true>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch });
	}

	[[nodiscard]]
	constexpr auto split_terminator(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_terminator_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	[[nodiscard]]
	constexpr auto split_terminator(View sv) const& noexcept
	{
		return utf8_split_view<View, true>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() });
	}

	[[nodiscard]]
	constexpr auto split_terminator(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_terminator_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split_terminator(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_split_view<View, true, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred));
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split_terminator(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_terminator_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	[[nodiscard]]
	constexpr auto rsplit_terminator(utf8_char ch) const& noexcept
	{
		return std::views::reverse(split_terminator(ch));
	}

	[[nodiscard]]
	constexpr auto rsplit_terminator(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplit_terminator_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	[[nodiscard]]
	constexpr auto rsplit_terminator(View sv) const& noexcept
	{
		return std::views::reverse(split_terminator(sv));
	}

	[[nodiscard]]
	constexpr auto rsplit_terminator(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplit_terminator_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto rsplit_terminator(Pred pred) const& noexcept
	{
		return std::views::reverse(split_terminator(std::move(pred)));
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto rsplit_terminator(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplit_terminator_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	[[nodiscard]]
	constexpr auto splitn(size_type count, utf8_char ch) const& noexcept
	{
		return utf8_splitn_char_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch },
			count);
	}

	[[nodiscard]]
	constexpr auto splitn(size_type count, utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::splitn_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ count, ch }
		};
	}

	[[nodiscard]]
	constexpr auto splitn(size_type count, View sv) const& noexcept
	{
		return utf8_splitn_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() },
			count);
	}

	[[nodiscard]]
	constexpr auto splitn(size_type count, View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::splitn_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ count, sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto splitn(size_type count, Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_splitn_view<View, false, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred),
			count);
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto splitn(size_type count, Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::splitn_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ count, std::move(pred) }
		};
	}

	[[nodiscard]]
	constexpr auto split_inclusive(utf8_char ch) const& noexcept
	{
		return utf8_split_inclusive_char_view<View>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch });
	}

	[[nodiscard]]
	constexpr auto split_inclusive(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_inclusive_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	[[nodiscard]]
	constexpr auto split_inclusive(View sv) const& noexcept
	{
		return utf8_split_inclusive_view<View>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() });
	}

	[[nodiscard]]
	constexpr auto split_inclusive(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_inclusive_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split_inclusive(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_split_inclusive_view<View, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred));
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr auto split_inclusive(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::split_inclusive_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	constexpr auto rsplitn(size_type count, utf8_char ch) const& noexcept
	{
		return utf8_splitn_char_view<View, true>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch },
			count);
	}

	constexpr auto rsplitn(size_type count, utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplitn_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ count, ch }
		};
	}

	constexpr auto rsplitn(size_type count, View sv) const& noexcept
	{
		return utf8_splitn_view<View, true>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() },
			count);
	}

	constexpr auto rsplitn(size_type count, View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplitn_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ count, sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto rsplitn(size_type count, Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_splitn_view<View, true, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred),
			count);
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto rsplitn(size_type count, Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rsplitn_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ count, std::move(pred) }
		};
	}

	constexpr auto matches(utf8_char ch) const& noexcept
	{
		return utf8_match_indices_char_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch })
			| std::views::values;
	}

	constexpr auto matches(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::matches_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	constexpr auto matches(View sv) const& noexcept
	{
		return utf8_match_indices_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() })
			| std::views::values;
	}

	constexpr auto matches(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::matches_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto matches(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_match_indices_view<View, false, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred))
			| std::views::values;
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto matches(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::matches_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto matches(R&& chars) const& noexcept(noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
	{
		return matches(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars)));
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto matches(R&& chars) && noexcept(std::is_nothrow_move_constructible_v<Derived>
		&& noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
		requires (!std::same_as<Derived, View>)
	{
		auto matcher = details::make_char_set_matcher<utf8_char>(std::forward<R>(chars));
		using accessor_type = details::matches_view_accessor<decltype(matcher)>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(matcher) }
		};
	}

	constexpr auto match_indices(utf8_char ch) const& noexcept
	{
		return utf8_match_indices_char_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch });
	}

	constexpr auto match_indices(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::match_indices_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	constexpr auto match_indices(View sv) const& noexcept
	{
		return utf8_match_indices_view<View, false>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() });
	}

	constexpr auto match_indices(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::match_indices_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto match_indices(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_match_indices_view<View, false, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred));
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto match_indices(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::match_indices_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto match_indices(R&& chars) const& noexcept(noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
	{
		return match_indices(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars)));
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto match_indices(R&& chars) && noexcept(std::is_nothrow_move_constructible_v<Derived>
		&& noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
		requires (!std::same_as<Derived, View>)
	{
		auto matcher = details::make_char_set_matcher<utf8_char>(std::forward<R>(chars));
		using accessor_type = details::match_indices_view_accessor<decltype(matcher)>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(matcher) }
		};
	}

	constexpr auto rmatches(utf8_char ch) const& noexcept
	{
		return rmatch_indices(ch) | std::views::values;
	}

	constexpr auto rmatches(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rmatches_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	constexpr auto rmatches(View sv) const& noexcept
	{
		return rmatch_indices(sv) | std::views::values;
	}

	constexpr auto rmatches(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rmatches_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto rmatches(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_match_indices_view<View, true, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred))
			| std::views::values;
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto rmatches(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rmatches_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto rmatches(R&& chars) const& noexcept(noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
	{
		return rmatches(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars)));
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto rmatches(R&& chars) && noexcept(std::is_nothrow_move_constructible_v<Derived>
		&& noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
		requires (!std::same_as<Derived, View>)
	{
		auto matcher = details::make_char_set_matcher<utf8_char>(std::forward<R>(chars));
		using accessor_type = details::rmatches_view_accessor<decltype(matcher)>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(matcher) }
		};
	}

	constexpr auto rmatch_indices(utf8_char ch) const& noexcept
	{
		return utf8_match_indices_char_view<View, true>::from_delimiter_storage(
			byte_view(),
			details::owned_utf8_split_char_delimiter{ ch });
	}

	constexpr auto rmatch_indices(utf8_char ch) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rmatch_indices_view_accessor<utf8_char>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ ch }
		};
	}

	constexpr auto rmatch_indices(View sv) const& noexcept
	{
		return utf8_match_indices_view<View, true>::from_delimiter_storage(
			byte_view(),
			details::borrowed_utf8_split_delimiter{ sv.base() });
	}

	constexpr auto rmatch_indices(View sv) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rmatch_indices_view_accessor<View>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ sv }
		};
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto rmatch_indices(Pred pred) const& noexcept
	{
		return details::basic_utf8_predicate_match_indices_view<View, true, std::remove_cvref_t<Pred>>::from_predicate(
			byte_view(),
			std::move(pred));
	}

	template <details::utf8_char_predicate Pred>
	constexpr auto rmatch_indices(Pred pred) && noexcept(std::is_nothrow_move_constructible_v<Derived>)
		requires (!std::same_as<Derived, View>)
	{
		using accessor_type = details::rmatch_indices_view_accessor<std::remove_cvref_t<Pred>>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(pred) }
		};
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto rmatch_indices(R&& chars) const& noexcept(noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
	{
		return rmatch_indices(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars)));
	}

	template <std::ranges::viewable_range R>
		requires details::char_set_viewable_range<R, utf8_char>
	constexpr auto rmatch_indices(R&& chars) && noexcept(std::is_nothrow_move_constructible_v<Derived>
		&& noexcept(details::make_char_set_matcher<utf8_char>(std::forward<R>(chars))))
		requires (!std::same_as<Derived, View>)
	{
		auto matcher = details::make_char_set_matcher<utf8_char>(std::forward<R>(chars));
		using accessor_type = details::rmatch_indices_view_accessor<decltype(matcher)>;
		return details::owning_string_view<Derived, accessor_type>{
			std::move(static_cast<Derived&>(*this)),
			accessor_type{ std::move(matcher) }
		};
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> split_once(utf8_char ch) const noexcept
	{
		return split_once(View::from_bytes_unchecked(details::utf8_char_view(ch)));
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> split_once(View sv) const noexcept
	{
		const auto delimiter = sv.base();
		const auto pos = details::find_utf8_split_delimiter(byte_view(), delimiter, 0);
		if (pos == npos)
		{
			return std::nullopt;
		}

		const auto bytes = byte_view();
		return std::pair{
			View::from_bytes_unchecked(bytes.substr(0, pos)),
			View::from_bytes_unchecked(bytes.substr(pos + delimiter.size()))
		};
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> split_once(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return std::nullopt;
		}

		if (chars.size() == 1)
		{
			return split_once(chars.front());
		}

		return split_once(details::utf8_char_span_matcher{ chars });
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> split_once(Pred pred) const noexcept
	{
		const auto match = details::find_utf8_predicate_match(byte_view(), 0, pred);
		if (match.pos == npos)
		{
			return std::nullopt;
		}

		const auto bytes = byte_view();
		return std::pair{
			View::from_bytes_unchecked(bytes.substr(0, match.pos)),
			View::from_bytes_unchecked(bytes.substr(match.pos + match.size))
		};
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> rsplit_once(utf8_char ch) const noexcept
	{
		return rsplit_once(View::from_bytes_unchecked(details::utf8_char_view(ch)));
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> rsplit_once(View sv) const noexcept
	{
		const auto delimiter = sv.base();
		const auto pos = details::rfind_utf8_split_delimiter(byte_view(), delimiter, byte_view().size());
		if (pos == npos)
		{
			return std::nullopt;
		}

		const auto bytes = byte_view();
		return std::pair{
			View::from_bytes_unchecked(bytes.substr(0, pos)),
			View::from_bytes_unchecked(bytes.substr(pos + delimiter.size()))
		};
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> rsplit_once(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return std::nullopt;
		}

		if (chars.size() == 1)
		{
			return rsplit_once(chars.front());
		}

		return rsplit_once(details::utf8_char_span_matcher{ chars });
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> rsplit_once(Pred pred) const noexcept
	{
		const auto match = details::rfind_utf8_predicate_match(byte_view(), byte_view().size(), pred);
		if (match.pos == npos)
		{
			return std::nullopt;
		}

		const auto bytes = byte_view();
		return std::pair{
			View::from_bytes_unchecked(bytes.substr(0, match.pos)),
			View::from_bytes_unchecked(bytes.substr(match.pos + match.size))
		};
	}

	[[nodiscard]]
	constexpr std::optional<std::pair<View, View>> split_once_at(size_type delim) const noexcept
	{
		if (!is_char_boundary(delim)) [[unlikely]]
		{
			return std::nullopt;
		}

		return split_once_at_unchecked(delim);
	}

	[[nodiscard]]
	constexpr std::pair<View, View> split_once_at_unchecked(size_type delim) const noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(is_char_boundary(delim));

		const auto bytes = byte_view();
		return {
			View::from_bytes_unchecked(bytes.substr(0, delim)),
			View::from_bytes_unchecked(bytes.substr(delim))
		};
	}

	[[nodiscard]] constexpr basic_utf8_string<> replace_all(utf8_char from, utf8_char to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_all(utf8_char from, View to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_all(View from, utf8_char to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_all(View from, View to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_all(std::span<const utf8_char> from, utf8_char to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_all(std::span<const utf8_char> from, View to) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(utf8_char from, utf8_char to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(utf8_char from, View to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(View from, utf8_char to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(View from, View to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(std::span<const utf8_char> from, utf8_char to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(std::span<const utf8_char> from, View to, const Allocator& alloc) const;

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr basic_utf8_string<> replace_all(Pred pred, utf8_char to) const;

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr basic_utf8_string<> replace_all(Pred pred, View to) const;

	template <details::utf8_char_predicate Pred, typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(Pred pred, utf8_char to, const Allocator& alloc) const;

	template <details::utf8_char_predicate Pred, typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_all(Pred pred, View to, const Allocator& alloc) const;

	[[nodiscard]] constexpr basic_utf8_string<> replace_n(size_type count, utf8_char from, utf8_char to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_n(size_type count, utf8_char from, View to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_n(size_type count, View from, utf8_char to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_n(size_type count, View from, View to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_n(size_type count, std::span<const utf8_char> from, utf8_char to) const;
	[[nodiscard]] constexpr basic_utf8_string<> replace_n(size_type count, std::span<const utf8_char> from, View to) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, utf8_char from, utf8_char to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, utf8_char from, View to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, View from, utf8_char to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, View from, View to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, std::span<const utf8_char> from, utf8_char to, const Allocator& alloc) const;

	template <typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, std::span<const utf8_char> from, View to, const Allocator& alloc) const;

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr basic_utf8_string<> replace_n(size_type count, Pred pred, utf8_char to) const;

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr basic_utf8_string<> replace_n(size_type count, Pred pred, View to) const;

	template <details::utf8_char_predicate Pred, typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, Pred pred, utf8_char to, const Allocator& alloc) const;

	template <details::utf8_char_predicate Pred, typename Allocator>
	[[nodiscard]]
	constexpr basic_utf8_string<Allocator> replace_n(size_type count, Pred pred, View to, const Allocator& alloc) const;

	[[nodiscard]]
	constexpr std::optional<View> strip_prefix(utf8_char ch) const noexcept
	{
		return strip_prefix(View::from_bytes_unchecked(details::utf8_char_view(ch)));
	}

	[[nodiscard]]
	constexpr std::optional<View> strip_prefix(View sv) const noexcept
	{
		if (!starts_with(sv))
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(sv.base().size()));
	}

	[[nodiscard]]
	constexpr std::optional<View> strip_suffix(utf8_char ch) const noexcept
	{
		return strip_suffix(View::from_bytes_unchecked(details::utf8_char_view(ch)));
	}

	[[nodiscard]]
	constexpr std::optional<View> strip_suffix(View sv) const noexcept
	{
		if (!ends_with(sv))
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(0, size() - sv.base().size()));
	}

	[[nodiscard]]
	constexpr std::optional<View> strip_circumfix(utf8_char prefix, utf8_char suffix) const noexcept
	{
		return strip_circumfix(
			View::from_bytes_unchecked(details::utf8_char_view(prefix)),
			View::from_bytes_unchecked(details::utf8_char_view(suffix)));
	}

	[[nodiscard]]
	constexpr std::optional<View> strip_circumfix(View prefix, View suffix) const noexcept
	{
		const auto stripped = strip_prefix(prefix);
		if (!stripped.has_value())
		{
			return std::nullopt;
		}

		return stripped->strip_suffix(suffix);
	}

	[[nodiscard]]
	constexpr View trim_prefix(utf8_char ch) const noexcept
	{
		return trim_prefix(View::from_bytes_unchecked(details::utf8_char_view(ch)));
	}

	[[nodiscard]]
	constexpr View trim_prefix(View sv) const noexcept
	{
		return strip_prefix(sv).value_or(view_from_whole_string());
	}

	[[nodiscard]]
	constexpr View trim_suffix(utf8_char ch) const noexcept
	{
		return trim_suffix(View::from_bytes_unchecked(details::utf8_char_view(ch)));
	}

	[[nodiscard]]
	constexpr View trim_suffix(View sv) const noexcept
	{
		return strip_suffix(sv).value_or(view_from_whole_string());
	}

	[[nodiscard]]
	constexpr View trim_start_matches(utf8_char ch) const noexcept
	{
		const auto pos = find_first_not_of(ch);
		return pos == npos
			? empty_view()
			: View::from_bytes_unchecked(byte_view().substr(pos));
	}

	[[nodiscard]]
	constexpr View trim_start_matches(View sv) const noexcept
	{
		auto result = byte_view();
		const auto needle = sv.base();
		if (needle.empty())
		{
			return view_from_whole_string();
		}

		while (result.starts_with(needle))
		{
			result.remove_prefix(needle.size());
		}

		return View::from_bytes_unchecked(result);
	}

	[[nodiscard]]
	constexpr View trim_start_matches(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return view_from_whole_string();
		}

		if (chars.size() == 1)
		{
			return trim_start_matches(chars.front());
		}

		return trim_start_matches(details::utf8_char_span_matcher{ chars });
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr View trim_start_matches(Pred pred) const noexcept
	{
		std::size_t pos = 0;
		while (pos < byte_view().size())
		{
			const auto ch = details::utf8_char_from_bytes_at(byte_view(), pos);
			const auto count = ch.code_unit_count();
			if (!std::invoke(pred, ch))
			{
				break;
			}

			pos += count;
		}

		return View::from_bytes_unchecked(byte_view().substr(pos));
	}

	[[nodiscard]]
	constexpr View trim_end_matches(utf8_char ch) const noexcept
	{
		const auto pos = find_last_not_of(ch);
		if (pos == npos)
		{
			return empty_view();
		}

		return View::from_bytes_unchecked(byte_view().substr(0, pos + char_at_unchecked(pos).code_unit_count()));
	}

	[[nodiscard]]
	constexpr View trim_end_matches(View sv) const noexcept
	{
		auto result = byte_view();
		const auto needle = sv.base();
		if (needle.empty())
		{
			return view_from_whole_string();
		}

		while (result.ends_with(needle))
		{
			result.remove_suffix(needle.size());
		}

		return View::from_bytes_unchecked(result);
	}

	[[nodiscard]]
	constexpr View trim_end_matches(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return view_from_whole_string();
		}

		if (chars.size() == 1)
		{
			return trim_end_matches(chars.front());
		}

		return trim_end_matches(details::utf8_char_span_matcher{ chars });
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr View trim_end_matches(Pred pred) const noexcept
	{
		std::size_t end = byte_view().size();
		while (end != 0)
		{
			const auto pos = details::previous_utf8_scalar_boundary(byte_view(), end);
			const auto ch = details::utf8_char_from_bytes_at(byte_view(), pos);
			if (!std::invoke(pred, ch))
			{
				break;
			}

			end = pos;
		}

		return View::from_bytes_unchecked(byte_view().substr(0, end));
	}

	[[nodiscard]]
	constexpr View trim_matches(utf8_char ch) const noexcept
	{
		return trim_start_matches(ch).trim_end_matches(ch);
	}

	[[nodiscard]]
	constexpr View trim_matches(View sv) const noexcept
	{
		return trim_start_matches(sv).trim_end_matches(sv);
	}

	[[nodiscard]]
	constexpr View trim_matches(std::span<const utf8_char> chars) const noexcept
	{
		return trim_start_matches(chars).trim_end_matches(chars);
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr View trim_matches(Pred pred) const noexcept
	{
		return trim_start_matches(pred).trim_end_matches(pred);
	}

	[[nodiscard]]
	constexpr View trim_start() const noexcept
	{
		const auto pos = details::find_utf8_non_whitespace_boundary(byte_view(), 0, false);
		return pos == npos
			? empty_view()
			: View::from_bytes_unchecked(byte_view().substr(pos));
	}

	[[nodiscard]]
	constexpr View trim_end() const noexcept
	{
		return View::from_bytes_unchecked(byte_view().substr(0, details::utf8_trim_end_boundary(byte_view(), false)));
	}

	constexpr View trim() const noexcept
	{
		return trim_start().trim_end();
	}

	[[nodiscard]]
	constexpr View trim_ascii_start() const noexcept
	{
		const auto pos = details::find_utf8_non_whitespace_boundary(byte_view(), 0, true);
		return pos == npos
			? empty_view()
			: View::from_bytes_unchecked(byte_view().substr(pos));
	}

	[[nodiscard]]
	constexpr View trim_ascii_end() const noexcept
	{
		return View::from_bytes_unchecked(byte_view().substr(0, details::utf8_trim_end_boundary(byte_view(), true)));
	}

	[[nodiscard]]
	constexpr View trim_ascii() const noexcept
	{
		return trim_ascii_start().trim_ascii_end();
	}

	[[nodiscard]]
	constexpr std::optional<utf8_char> char_at(size_type index) const noexcept
	{
		if (index >= size() || !is_char_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		return char_at_unchecked(index);
	}

	[[nodiscard]]
	constexpr utf8_char char_at_unchecked(size_type index) const noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(index < size());
		UTF8_RANGES_DEBUG_ASSERT(is_char_boundary(index));

		const auto bytes = byte_view();
		const auto len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[index]));
		return utf8_char::from_utf8_bytes_unchecked(bytes.data() + index, len);
	}

	[[nodiscard]]
	constexpr std::optional<View> grapheme_at(size_type index) const noexcept
	{
		const auto bytes = byte_view();
		if (index >= bytes.size() || !details::is_grapheme_boundary(bytes, index)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = details::next_grapheme_boundary(bytes, index);
		return View::from_bytes_unchecked(std::u8string_view{ bytes.data() + index, end - index });
	}

	[[nodiscard]]
	constexpr std::optional<View> substr(size_type pos, size_type count = npos) const noexcept
	{
		const auto bytes = byte_view();
		if (pos > bytes.size()) [[unlikely]]
		{
			return std::nullopt;
		}

		if (pos != 0 && pos != bytes.size()
			&& !details::is_utf8_lead_byte(static_cast<std::uint8_t>(bytes[pos]))) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto remaining = bytes.size() - pos;
		const auto length = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + length;

		if (end != 0 && end != bytes.size()
			&& !details::is_utf8_lead_byte(static_cast<std::uint8_t>(bytes[end]))) [[unlikely]]
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(std::u8string_view{ bytes.data() + pos, end - pos });
	}

	[[nodiscard]]
	constexpr std::optional<View> grapheme_substr(size_type pos, size_type count = npos) const noexcept
	{
		const auto bytes = byte_view();
		if (!details::is_grapheme_boundary(bytes, pos)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto remaining = bytes.size() - pos;
		const auto length = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + length;

		if (!details::is_grapheme_boundary(bytes, end)) [[unlikely]]
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(std::u8string_view{ bytes.data() + pos, end - pos });
	}

	[[nodiscard]]
	constexpr std::optional<utf8_char> front() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return front_unchecked();
	}

	[[nodiscard]]
	constexpr utf8_char front_unchecked() const noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(!empty());
		return *chars().begin();
	}

	[[nodiscard]]
	constexpr std::optional<utf8_char> back() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return back_unchecked();
	}

	[[nodiscard]]
	constexpr utf8_char back_unchecked() const noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(!empty());
		return *reversed_chars().begin();
	}

	[[nodiscard]]
	constexpr bool starts_with(char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	[[nodiscard]]
	constexpr bool starts_with(char8_t ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	[[nodiscard]]
	constexpr bool starts_with(utf8_char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	[[nodiscard]]
	constexpr bool starts_with(View sv) const noexcept
	{
		return byte_view().starts_with(sv.base());
	}

	[[nodiscard]]
	constexpr bool starts_with(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return false;
		}

		if (chars.size() == 1)
		{
			return starts_with(chars.front());
		}

		return starts_with(details::utf8_char_span_matcher{ chars });
	}

	template <details::utf8_char_predicate Pred>
	[[nodiscard]]
	constexpr bool starts_with(Pred pred) const
		noexcept(noexcept(std::invoke(std::declval<const std::remove_cvref_t<Pred>&>(), std::declval<utf8_char>())))
	{
		return !empty() && static_cast<bool>(std::invoke(pred, front_unchecked()));
	}

	[[nodiscard]]
	constexpr bool ends_with(char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	[[nodiscard]]
	constexpr bool ends_with(char8_t ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	[[nodiscard]]
	constexpr bool ends_with(utf8_char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	[[nodiscard]]
	constexpr bool ends_with(View sv) const noexcept
	{
		return byte_view().ends_with(sv.base());
	}

	[[nodiscard]]
	constexpr bool ends_with(std::span<const utf8_char> chars) const noexcept
	{
		if (chars.empty())
		{
			return false;
		}

		if (chars.size() == 1)
		{
			return ends_with(chars.front());
		}

		return !empty() && details::utf8_char_span_matcher{ chars }(back_unchecked());
	}

	[[nodiscard]]
	constexpr size_type ceil_char_boundary(size_type pos) const noexcept
	{
		pos = (std::min)(size(), pos);
		while (pos != size() && !is_char_boundary(pos))
		{
			++pos;
		}

		return pos;
	}

	[[nodiscard]]
	constexpr size_type floor_char_boundary(size_type pos) const noexcept
	{
		pos = (std::min)(size(), pos);
		while (pos != 0 && !is_char_boundary(pos))
		{
			--pos;
		}

		return pos;
	}

	[[nodiscard]]
	constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept
	{
		return details::ceil_grapheme_boundary(byte_view(), pos);
	}

	[[nodiscard]]
	constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept
	{
		return details::floor_grapheme_boundary(byte_view(), pos);
	}

protected:
	constexpr const Derived& self() const noexcept
	{
		return static_cast<const Derived&>(*this);
	}

	constexpr View view_from_whole_string() const noexcept
	{
		return View::from_bytes_unchecked(byte_view());
	}

	constexpr View empty_view() const noexcept
	{
		return View::from_bytes_unchecked(byte_view().substr(0, 0));
	}

	constexpr std::u8string_view byte_view() const noexcept
	{
		return std::u8string_view{ self().base() };
	}
};

}

}

namespace std::ranges
{
	template <>
	inline constexpr bool enable_borrowed_range<unicode_ranges::details::utf8_char_indices_view> = true;

	template <typename View>
	inline constexpr bool enable_borrowed_range<unicode_ranges::details::utf8_grapheme_indices_view<View>> = true;

	template <typename View, bool DropTrailingEmpty>
	inline constexpr bool enable_borrowed_range<
		unicode_ranges::details::basic_utf8_split_view<
			View,
			DropTrailingEmpty,
			unicode_ranges::details::borrowed_utf8_split_delimiter>> = true;

	template <typename View, bool Reverse>
	inline constexpr bool enable_borrowed_range<
		unicode_ranges::details::basic_utf8_splitn_view<
			View,
			Reverse,
			unicode_ranges::details::borrowed_utf8_split_delimiter>> = true;

	template <typename View>
	inline constexpr bool enable_borrowed_range<
		unicode_ranges::details::basic_utf8_split_inclusive_view<
			View,
			unicode_ranges::details::borrowed_utf8_split_delimiter>> = true;

	template <typename View, bool Reverse>
	inline constexpr bool enable_borrowed_range<
		unicode_ranges::details::basic_utf8_match_indices_view<
			View,
			Reverse,
			unicode_ranges::details::borrowed_utf8_split_delimiter>> = true;

	template <typename View>
	inline constexpr bool enable_borrowed_range<
		unicode_ranges::details::basic_utf8_split_trimmed_view<
			View,
			unicode_ranges::details::borrowed_utf8_split_delimiter>> = true;

	template <typename View, bool AsciiOnly>
	inline constexpr bool enable_borrowed_range<unicode_ranges::details::utf8_whitespace_split_view<View, AsciiOnly>> = true;
}

#endif // UTF8_RANGES_UTF8_STRING_CRTP_HPP
