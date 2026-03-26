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
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + code_units.size() * details::encoding_constants::three_code_unit_count,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					const auto scalar = details::decode_valid_utf16_char(code_units.substr(read_index, count));
					write_index += details::encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

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
		const auto code_units = rg.base();
		replacement.resize_and_overwrite(code_units.size() * details::encoding_constants::three_code_unit_count,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					const auto scalar = details::decode_valid_utf16_char(code_units.substr(read_index, count));
					write_index += details::encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});
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
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(bytes.substr(read_index, count));
					write_index += details::encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

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
		const auto bytes = rg.base();
		replacement.resize_and_overwrite(bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(bytes.substr(read_index, count));
					write_index += details::encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});
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
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_utf8_owned(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>{ View::from_bytes_unchecked(byte_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf8_string_crtp<Derived, View>::to_utf16(const Allocator& alloc) const
	{
		basic_utf16_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, utf8_char to) const
	{
		return replace_all(
			View::from_bytes_unchecked(from.as_view()),
			View::from_bytes_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, View to) const
	{
		return replace_all(View::from_bytes_unchecked(from.as_view()), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(View from, utf8_char to) const
	{
		return replace_all(from, View::from_bytes_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(View from, View to) const
	{
		return replace_all(from, to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(
			View::from_bytes_unchecked(from.as_view()),
			View::from_bytes_unchecked(to.as_view()),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_bytes_unchecked(from.as_view()), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(View from, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_bytes_unchecked(to.as_view()), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(View from, View to, const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>::from_bytes_unchecked(details::replace_utf8_bytes_copy(
			byte_view(),
			from.base(),
			to.base(),
			npos,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, utf8_char to) const
	{
		return replace_n(
			count,
			View::from_bytes_unchecked(from.as_view()),
			View::from_bytes_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, View to) const
	{
		return replace_n(count, View::from_bytes_unchecked(from.as_view()), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, utf8_char to) const
	{
		return replace_n(count, from, View::from_bytes_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, View to) const
	{
		return replace_n(count, from, to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(
			count,
			View::from_bytes_unchecked(from.as_view()),
			View::from_bytes_unchecked(to.as_view()),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_bytes_unchecked(from.as_view()), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_bytes_unchecked(to.as_view()), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, View to, const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>::from_bytes_unchecked(details::replace_utf8_bytes_copy(
			byte_view(),
			from.base(),
			to.base(),
			count,
			alloc));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_utf16_owned(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>{ View::from_code_units_unchecked(code_unit_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf16_string_crtp<Derived, View>::to_utf8(const Allocator& alloc) const
	{
		basic_utf8_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, utf16_char to) const
	{
		return replace_all(
			View::from_code_units_unchecked(from.as_view()),
			View::from_code_units_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, View to) const
	{
		return replace_all(View::from_code_units_unchecked(from.as_view()), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(View from, utf16_char to) const
	{
		return replace_all(from, View::from_code_units_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(View from, View to) const
	{
		return replace_all(from, to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(
			View::from_code_units_unchecked(from.as_view()),
			View::from_code_units_unchecked(to.as_view()),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_code_units_unchecked(from.as_view()), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(View from, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_code_units_unchecked(to.as_view()), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(View from, View to, const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>::from_code_units_unchecked(details::replace_utf16_code_units_copy(
			code_unit_view(),
			from.base(),
			to.base(),
			npos,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, utf16_char to) const
	{
		return replace_n(
			count,
			View::from_code_units_unchecked(from.as_view()),
			View::from_code_units_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, View to) const
	{
		return replace_n(count, View::from_code_units_unchecked(from.as_view()), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, utf16_char to) const
	{
		return replace_n(count, from, View::from_code_units_unchecked(to.as_view()));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, View to) const
	{
		return replace_n(count, from, to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(
			count,
			View::from_code_units_unchecked(from.as_view()),
			View::from_code_units_unchecked(to.as_view()),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_code_units_unchecked(from.as_view()), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_code_units_unchecked(to.as_view()), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, View to, const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>::from_code_units_unchecked(details::replace_utf16_code_units_copy(
			code_unit_view(),
			from.base(),
			to.base(),
			count,
			alloc));
	}
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_char::to_utf8_owned(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>{ as_utf8_view(), alloc };
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_char::to_utf16_owned(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>{
			utf16_string_view::from_code_units_unchecked(as_view()),
			alloc
		};
	}

}

#endif // UTF8_RANGES_TRANSCODING_HPP
