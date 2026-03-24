#if defined(UTF8_RANGES_UTF8_STRING_HPP) && defined(UTF8_RANGES_UTF16_STRING_HPP) && !defined(UTF8_RANGES_TRANSCODING_HPP)
#define UTF8_RANGES_TRANSCODING_HPP

namespace unicode_ranges
{
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::append_range(views::utf8_view rg)
	{
		return append_bytes(rg.base());
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::append_range(views::utf16_view rg)
	{
		const auto code_units = rg.base();
		base_.reserve(base_.size() + code_units.size() * 3u);
		for (utf16_char ch : rg)
		{
			std::array<char8_t, 4> encoded{};
			const auto encoded_count = ch.encode_utf8<char8_t>(encoded.begin());
			base_.append(encoded.data(), encoded.data() + encoded_count);
		}

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::assign_range(views::utf8_view rg)
	{
		const auto bytes = rg.base();
		if (overlaps_base(bytes))
		{
			base_type replacement{ bytes, base_.get_allocator() };
			base_ = std::move(replacement);
		}
		else
		{
			base_.assign(bytes);
		}

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::assign_range(views::utf16_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		replacement.reserve(rg.base().size() * 3u);
		for (utf16_char ch : rg)
		{
			std::array<char8_t, 4> encoded{};
			const auto encoded_count = ch.encode_utf8<char8_t>(encoded.begin());
			replacement.append(encoded.data(), encoded.data() + encoded_count);
		}
		base_ = std::move(replacement);
		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>::basic_utf8_string(utf16_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::operator+=(utf16_string_view sv)
	{
		return append_range(sv.chars());
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::append_range(views::utf16_view rg)
	{
		return append_code_units(rg.base());
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::append_range(views::utf8_view rg)
	{
		const auto bytes = rg.base();
		base_.reserve(base_.size() + bytes.size());
		for (utf8_char ch : rg)
		{
			std::array<char16_t, 2> encoded{};
			const auto encoded_count = ch.encode_utf16<char16_t>(encoded.begin());
			base_.append(encoded.data(), encoded.data() + encoded_count);
		}

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::assign_range(views::utf16_view rg)
	{
		const auto code_units = rg.base();
		if (overlaps_base(code_units))
		{
			base_type replacement{ code_units, base_.get_allocator() };
			base_ = std::move(replacement);
		}
		else
		{
			base_.assign(code_units);
		}

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::assign_range(views::utf8_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		replacement.reserve(rg.base().size());
		for (utf8_char ch : rg)
		{
			std::array<char16_t, 2> encoded{};
			const auto encoded_count = ch.encode_utf16<char16_t>(encoded.begin());
			replacement.append(encoded.data(), encoded.data() + encoded_count);
		}
		base_ = std::move(replacement);
		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>::basic_utf16_string(utf8_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::operator+=(utf8_string_view sv)
	{
		return append_range(sv.chars());
	}

	namespace details
	{
	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf8_string_crtp<Derived, View>::to_utf16(const Allocator& alloc) const
	{
		basic_utf16_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf16_string_crtp<Derived, View>::to_utf8(const Allocator& alloc) const
	{
		basic_utf8_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}
	}

}

#endif // UTF8_RANGES_TRANSCODING_HPP
