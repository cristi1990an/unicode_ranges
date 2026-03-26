#ifndef UTF8_RANGES_UTF16_STRING_HPP
#define UTF8_RANGES_UTF16_STRING_HPP

#include "utf16_string_view.hpp"

namespace unicode_ranges
{

template <typename Allocator>
class basic_utf16_string : public details::utf16_string_crtp<basic_utf16_string<Allocator>, utf16_string_view>
{
	using crtp = details::utf16_string_crtp<basic_utf16_string<Allocator>, utf16_string_view>;
	using equivalent_utf16_string_view = utf16_string_view;
	using equivalent_string_view = std::u16string_view;

public:
	using base_type = std::basic_string<char16_t, std::char_traits<char16_t>, Allocator>;
	using allocator_type = Allocator;
	using value_type = utf16_char;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	static constexpr auto from_bytes(std::string_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> std::expected<basic_utf16_string, utf8_error>
	{
		if (auto transcoded = details::transcode_utf8_to_utf16_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_code_units_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> std::expected<basic_utf16_string, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		if (auto copied = details::copy_validated_utf16_code_units(bytes, alloc); copied) [[likely]]
		{
			return from_code_units_unchecked(std::move(*copied));
		}

		else
		{
			return std::unexpected(copied.error());
		}
	}

	static constexpr auto from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> std::expected<basic_utf16_string, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		if (auto transcoded = details::transcode_unicode_scalars_to_utf16_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_code_units_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(base_type&& bytes) noexcept
		-> std::expected<basic_utf16_string, utf16_error>
	{
		if (auto validation = details::validate_utf16(equivalent_string_view{ bytes }); !validation) [[unlikely]]
		{
			return std::unexpected(validation.error());
		}

		return from_code_units_unchecked(std::move(bytes));
	}

	static constexpr basic_utf16_string from_code_units_unchecked(base_type code_units) noexcept
	{
		return basic_utf16_string{ std::move(code_units) };
	}

	static constexpr basic_utf16_string from_code_units_unchecked(base_type code_units, const Allocator& alloc) noexcept
	{
		return from_code_units_unchecked(equivalent_string_view{ code_units }, alloc);
	}

	static constexpr basic_utf16_string from_code_units_unchecked(
		equivalent_string_view code_units,
		const Allocator& alloc = Allocator()) noexcept
	{
		basic_utf16_string result;
		result.base_ = base_type{ code_units, alloc };
		return result;
	}

	static constexpr basic_utf16_string from_bytes_unchecked(base_type&& bytes) noexcept
	{
		return from_code_units_unchecked(std::move(bytes));
	}

	static constexpr auto from_bytes_unchecked(std::string_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> basic_utf16_string
	{
		base_type utf16_code_units{ alloc };
		utf16_code_units.resize_and_overwrite(bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(
						std::basic_string_view<char>{ bytes.data() + read_index, count });
					write_index += details::encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

		return from_code_units_unchecked(std::move(utf16_code_units));
	}

	static constexpr auto from_bytes_unchecked(std::wstring_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> basic_utf16_string
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			base_type result{ alloc };
			result.resize_and_overwrite(bytes.size(),
				[&](char16_t* buffer, std::size_t) noexcept
				{
					for (std::size_t index = 0; index != bytes.size(); ++index)
					{
						buffer[index] = static_cast<char16_t>(bytes[index]);
					}

					return bytes.size();
				});

			return from_code_units_unchecked(std::move(result));
		}

		base_type utf16_code_units{ alloc };
		utf16_code_units.resize_and_overwrite(bytes.size() * details::encoding_constants::utf16_surrogate_code_unit_count,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (wchar_t ch : bytes)
				{
					write_index += details::encode_unicode_scalar_utf16_unchecked(static_cast<std::uint32_t>(ch), buffer + write_index);
				}

				return write_index;
			});

		return from_code_units_unchecked(std::move(utf16_code_units));
	}

private:
	[[nodiscard]]
	constexpr bool overlaps_base(equivalent_string_view code_units) const noexcept
	{
		if (code_units.empty() || base_.empty())
		{
			return false;
		}

		std::less<const char16_t*> less{};
		const auto* base_begin = base_.data();
		const auto* base_end = base_begin + base_.size();
		const auto* source_begin = code_units.data();
		const auto* source_end = source_begin + code_units.size();
		return less(base_begin, source_end) && less(source_begin, base_end);
	}

	[[nodiscard]]
	constexpr size_type overlap_offset(equivalent_string_view code_units) const noexcept
	{
		return static_cast<size_type>(code_units.data() - base_.data());
	}

	constexpr basic_utf16_string& append_code_units(equivalent_string_view code_units)
	{
		if (overlaps_base(code_units))
		{
			const auto offset = overlap_offset(code_units);
			base_.append(base_, offset, code_units.size());
		}
		else
		{
			base_.append(code_units);
		}

		return *this;
	}

	constexpr basic_utf16_string& insert_code_units(size_type index, equivalent_string_view code_units)
	{
		if (overlaps_base(code_units))
		{
			const auto offset = overlap_offset(code_units);
			base_.insert(index, base_, offset, code_units.size());
		}
		else
		{
			base_.insert(index, code_units);
		}

		return *this;
	}

	constexpr basic_utf16_string& replace_code_units(size_type pos, size_type count, equivalent_string_view code_units)
	{
		if (overlaps_base(code_units))
		{
			const auto offset = overlap_offset(code_units);
			base_.replace(pos, count, base_, offset, code_units.size());
		}
		else
		{
			base_.replace(pos, count, code_units);
		}

		return *this;
	}

	template <typename ResultAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<ResultAllocator> replace_code_units_copy(
		equivalent_string_view needle,
		equivalent_string_view replacement,
		size_type count,
		const ResultAllocator& alloc) const
	{
		return basic_utf16_string<ResultAllocator>::from_code_units_unchecked(details::replace_utf16_code_units_copy(
			equivalent_string_view{ base_ },
			needle,
			replacement,
			count,
			alloc));
	}

	constexpr basic_utf16_string& replace_code_units_in_place(
		equivalent_string_view needle,
		equivalent_string_view replacement,
		size_type count)
	{
		if (needle.empty() || count == 0)
		{
			return *this;
		}

		base_type stable_needle{ base_.get_allocator() };
		if (overlaps_base(needle))
		{
			stable_needle.assign(needle);
			needle = stable_needle;
		}

		base_type stable_replacement{ base_.get_allocator() };
		if (overlaps_base(replacement))
		{
			stable_replacement.assign(replacement);
			replacement = stable_replacement;
		}

		size_type search_pos = 0;
		size_type replacements = 0;
		while (replacements != count)
		{
			const auto match = details::find_utf16_split_delimiter(equivalent_string_view{ base_ }, needle, search_pos);
			if (match == equivalent_string_view::npos)
			{
				break;
			}

			replace_code_units(match, needle.size(), replacement);
			search_pos = match + replacement.size();
			++replacements;
		}

		return *this;
	}

public:

	basic_utf16_string() = default;
	basic_utf16_string(const basic_utf16_string&) = default;
	basic_utf16_string(basic_utf16_string&&) = default;
	basic_utf16_string& operator=(const basic_utf16_string&) = default;
	basic_utf16_string& operator=(basic_utf16_string&&) = default;

	constexpr basic_utf16_string(const Allocator& alloc)
		: base_(alloc)
	{ }

	constexpr basic_utf16_string(const basic_utf16_string& other, const Allocator& alloc)
		: base_(other.base_, alloc)
	{ }

	constexpr basic_utf16_string(basic_utf16_string&& other, const Allocator& alloc)
		noexcept(std::allocator_traits<Allocator>::is_always_equal)
		: base_(std::move(other.base_), alloc)
	{ }

	constexpr basic_utf16_string(utf16_string_view view, const Allocator& alloc = Allocator())
		: base_(view.base(), alloc)
	{}

	constexpr basic_utf16_string(utf8_string_view view, const Allocator& alloc = Allocator());

	constexpr basic_utf16_string(std::size_t count, utf16_char ch, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(count, ch);
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append_range(std::forward<R>(rg));
	}

	constexpr basic_utf16_string(std::initializer_list<utf16_char> ilist, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(ilist);
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string(It it, Sent sent, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(std::move(it), std::move(sent));
	}

	constexpr basic_utf16_string& append_range(views::utf16_view rg);

	constexpr basic_utf16_string& append_range(views::utf8_view rg);

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& append_range(R&& rg)
	{
		base_type appended{ base_.get_allocator() };
		for (utf16_char ch : std::forward<R>(rg))
		{
			appended.append(ch.as_view());
		}
		base_.append(appended);
		return *this;
	}

	constexpr basic_utf16_string& assign_range(views::utf16_view rg);

	constexpr basic_utf16_string& assign_range(views::utf8_view rg);

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& assign_range(R&& rg)
	{
		base_type replacement{ base_.get_allocator() };
		for (utf16_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}
		base_ = std::move(replacement);
		return *this;
	}

	constexpr basic_utf16_string& append(size_type count, utf16_char ch)
	{
		const auto sv = ch.as_view();
		const auto total_size = sv.size() * count;
		const auto old_size = base_.size();

		base_.resize_and_overwrite(old_size + total_size,
			[&](char16_t* buffer, std::size_t)
			{
				buffer = buffer + old_size;
				for (size_type i = 0; i != count; ++i)
				{
					std::ranges::copy(sv, buffer);
					buffer += sv.size();
				}

				return total_size;
			});

		return *this;
	}

	constexpr basic_utf16_string& assign(size_type count, utf16_char ch)
	{
		base_.clear();
		return append(count, ch);
	}

	constexpr basic_utf16_string& append(utf16_string_view sv)
	{
		return append_code_units(sv.base());
	}

	constexpr basic_utf16_string& assign(utf16_string_view sv)
	{
		base_.assign(sv.base());
		return *this;
	}

	constexpr basic_utf16_string& assign(utf16_char ch)
	{
		base_.assign(ch.as_view());
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string& append(It it, Sent sent)
	{
		return append_range(std::ranges::subrange(std::move(it), std::move(sent)));
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string& assign(It it, Sent sent)
	{
		base_.clear();
		return append(std::move(it), std::move(sent));
	}

	constexpr basic_utf16_string& append(std::initializer_list<utf16_char> ilist)
	{
		return append_range(ilist);
	}

	constexpr basic_utf16_string& assign(std::initializer_list<utf16_char> ilist)
	{
		return assign_range(ilist);
	}

	constexpr basic_utf16_string& operator=(utf16_string_view sv)
	{
		return assign(sv);
	}

	constexpr basic_utf16_string& operator=(utf16_char ch)
	{
		return assign(ch);
	}

	constexpr basic_utf16_string& operator=(std::initializer_list<utf16_char> ilist)
	{
		return assign(ilist);
	}

	constexpr basic_utf16_string& operator+=(utf16_string_view sv)
	{
		return append(sv);
	}

	constexpr basic_utf16_string& operator+=(utf8_string_view sv);

	constexpr basic_utf16_string& operator+=(utf16_char ch)
	{
		push_back(ch);
		return *this;
	}

	constexpr basic_utf16_string& operator+=(utf8_char ch)
	{
		push_back(static_cast<utf16_char>(ch));
		return *this;
	}

	constexpr basic_utf16_string& operator+=(std::initializer_list<utf16_char> ilist)
	{
		return append(ilist);
	}

	constexpr void shrink_to_fit()
	{
		base_.shrink_to_fit();
	}

	[[nodiscard]]
	constexpr size_type capacity() const
	{
		return base_.capacity();
	}

	[[nodiscard]]
	constexpr allocator_type get_allocator() const noexcept
	{
		return base_.get_allocator();
	}

	[[nodiscard]]
	constexpr size_type size() const
	{
		return base_.size();
	}

	constexpr basic_utf16_string& insert(size_type index, utf16_string_view sv)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

		return insert_code_units(index, sv.base());
	}

	constexpr basic_utf16_string& insert(size_type index, utf16_char ch)
	{
		return insert(index, equivalent_utf16_string_view::from_code_units_unchecked(ch.as_view()));
	}

	constexpr basic_utf16_string& insert(size_type index, size_type count, utf16_char ch)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

		base_type inserted{ base_.get_allocator() };
		const auto sv = ch.as_view();
		for (size_type i = 0; i != count; ++i)
		{
			inserted.append(sv);
		}

		base_.insert(index, inserted);
		return *this;
	}

	constexpr basic_utf16_string& insert_range(size_type index, views::utf16_view rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

		return insert_code_units(index, rg.base());
	}

	constexpr basic_utf16_string& insert_range(size_type index, views::utf8_view rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

		base_type inserted{ base_.get_allocator() };
		inserted.reserve(rg.base().size());
		for (utf8_char ch : rg)
		{
			std::array<char16_t, 2> encoded{};
			const auto encoded_count = ch.encode_utf16<char16_t>(encoded.begin());
			inserted.append(encoded.data(), encoded.data() + encoded_count);
		}

		base_.insert(index, inserted);
		return *this;
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& insert_range(size_type index, R&& rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-16 character boundary");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf16_char_range
		{
			utf16_char ch;

			constexpr auto begin() const noexcept
			{
				return ch.as_view().begin();
			}

			constexpr auto end() const noexcept
			{
				return ch.as_view().end();
			}
		};

		auto inserted = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf16_char_range{ static_cast<utf16_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.insert_range(base_.begin() + static_cast<difference_type>(index), inserted);
#else
		const utf16_string inserted(std::from_range, std::forward<R>(rg));
		base_.insert(index, inserted.base());
#endif
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf16_string& insert(size_type index, It first, Sent last)
	{
		return insert_range(index, std::ranges::subrange(std::move(first), std::move(last)));
	}

	constexpr basic_utf16_string& insert(size_type index, std::initializer_list<utf16_char> ilist)
	{
		return insert_range(index, ilist);
	}

	constexpr std::optional<value_type> pop_back()
	{
		if (base_.empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto removed = this->back_unchecked();
		const auto code_units_to_remove = removed.code_unit_count();
		const auto where_idx = base_.size() - code_units_to_remove;
		base_.erase(where_idx, code_units_to_remove);
		return removed;
	}

	constexpr basic_utf16_string& erase(size_type index, size_type count = npos)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("erase index out of range");
		}

		const auto remaining = size() - index;
		const auto erase_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = index + erase_count;

		if (!this->is_char_boundary(index) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("erase range must be a valid UTF-16 substring");
		}

		base_.erase(index, erase_count);
		return *this;
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_char from, utf16_char to) const&
	{
		return replace_all(
			equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()),
			equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_char from, utf16_string_view to) const&
	{
		return replace_all(equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()), to);
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_char to) const&
	{
		return replace_all(from, equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_string_view to) const&
	{
		return replace_code_units_copy(from.base(), to.base(), npos, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_char from, utf16_char to) &&
	{
		return std::move(*this).replace_all(
			equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()),
			equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_char from, utf16_string_view to) &&
	{
		return std::move(*this).replace_all(
			equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()),
			to);
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_char to) &&
	{
		return std::move(*this).replace_all(
			from,
			equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_all(utf16_string_view from, utf16_string_view to) &&
	{
		replace_code_units_in_place(from.base(), to.base(), npos);
		return std::move(*this);
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_char from, utf16_char to) const&
	{
		return replace_n(
			count,
			equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()),
			equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_char from, utf16_string_view to) const&
	{
		return replace_n(count, equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()), to);
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_string_view from, utf16_char to) const&
	{
		return replace_n(count, from, equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_string_view from, utf16_string_view to) const&
	{
		return replace_code_units_copy(from.base(), to.base(), count, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_char from, utf16_char to) &&
	{
		return std::move(*this).replace_n(
			count,
			equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()),
			equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_char from, utf16_string_view to) &&
	{
		return std::move(*this).replace_n(
			count,
			equivalent_utf16_string_view::from_code_units_unchecked(from.as_view()),
			to);
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_string_view from, utf16_char to) &&
	{
		return std::move(*this).replace_n(
			count,
			from,
			equivalent_utf16_string_view::from_code_units_unchecked(to.as_view()));
	}

	[[nodiscard]]
	constexpr basic_utf16_string replace_n(size_type count, utf16_string_view from, utf16_string_view to) &&
	{
		replace_code_units_in_place(from.base(), to.base(), count);
		return std::move(*this);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_all(
		utf16_char from,
		utf16_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_all(
		utf16_char from,
		utf16_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_all(
		utf16_string_view from,
		utf16_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_all(
		utf16_string_view from,
		utf16_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <details::utf16_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_all(
		Pred pred,
		utf16_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<std::remove_cvref_t<Pred>, OtherAllocator>(
			std::move(pred),
			to,
			alloc);
	}

	template <details::utf16_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_all(
		Pred pred,
		utf16_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<std::remove_cvref_t<Pred>, OtherAllocator>(
			std::move(pred),
			to,
			alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_n(
		size_type count,
		utf16_char from,
		utf16_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_n(
		size_type count,
		utf16_char from,
		utf16_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_n(
		size_type count,
		utf16_string_view from,
		utf16_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_n(
		size_type count,
		utf16_string_view from,
		utf16_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <details::utf16_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_n(
		size_type count,
		Pred pred,
		utf16_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<std::remove_cvref_t<Pred>, OtherAllocator>(
			count,
			std::move(pred),
			to,
			alloc);
	}

	template <details::utf16_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf16_string<OtherAllocator> replace_n(
		size_type count,
		Pred pred,
		utf16_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<std::remove_cvref_t<Pred>, OtherAllocator>(
			count,
			std::move(pred),
			to,
			alloc);
	}

	constexpr basic_utf16_string& replace_inplace(size_type pos, size_type count, utf16_string_view other)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		const auto remaining = size() - pos;
		const auto replace_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + replace_count;

		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("replace range must be a valid UTF-16 substring");
		}

		return replace_code_units(pos, replace_count, other.base());
	}

	constexpr basic_utf16_string& replace_inplace(size_type pos, size_type count, utf16_char other)
	{
		return replace_inplace(pos, count, equivalent_utf16_string_view::from_code_units_unchecked(other.as_view()));
	}

	constexpr basic_utf16_string& replace_inplace(size_type pos, utf16_string_view other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_code_units(pos, replace_count, other.base());
	}

	constexpr basic_utf16_string& replace_inplace(size_type pos, utf16_char other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		base_.replace(pos, replace_count, other.as_view());
		return *this;
	}

	constexpr basic_utf16_string& replace_with_range_inplace(size_type pos, size_type count, views::utf16_view rg)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		const auto remaining = size() - pos;
		const auto replace_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + replace_count;

		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("replace range must be a valid UTF-16 substring");
		}

		return replace_code_units(pos, replace_count, rg.base());
	}

	constexpr basic_utf16_string& replace_with_range_inplace(size_type pos, size_type count, views::utf8_view rg)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		const auto remaining = size() - pos;
		const auto replace_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + replace_count;

		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("replace range must be a valid UTF-16 substring");
		}

		base_type replacement{ base_.get_allocator() };
		replacement.reserve(rg.base().size());
		for (utf8_char ch : rg)
		{
			std::array<char16_t, 2> encoded{};
			const auto encoded_count = ch.encode_utf16<char16_t>(encoded.begin());
			replacement.append(encoded.data(), encoded.data() + encoded_count);
		}

		base_.replace(pos, replace_count, replacement);
		return *this;
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& replace_with_range_inplace(size_type pos, size_type count, R&& rg)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		const auto remaining = size() - pos;
		const auto replace_count = (count == npos || count > remaining) ? remaining : count;
		const auto end = pos + replace_count;

		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("replace range must be a valid UTF-16 substring");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf16_char_range
		{
			utf16_char ch;

			constexpr auto begin() const noexcept
			{
				return ch.as_view().begin();
			}

			constexpr auto end() const noexcept
			{
				return ch.as_view().end();
			}
		};

		auto replacement = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf16_char_range{ static_cast<utf16_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(end),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf16_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	constexpr basic_utf16_string& replace_with_range_inplace(size_type pos, views::utf16_view rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_with_range_inplace(pos, replace_count, rg);
	}

	constexpr basic_utf16_string& replace_with_range_inplace(size_type pos, views::utf8_view rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_with_range_inplace(pos, replace_count, rg);
	}

	template <details::container_compatible_range<utf16_char> R>
	constexpr basic_utf16_string& replace_with_range_inplace(size_type pos, R&& rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-16 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf16_char_range
		{
			utf16_char ch;

			constexpr auto begin() const noexcept
			{
				return ch.as_view().begin();
			}

			constexpr auto end() const noexcept
			{
				return ch.as_view().end();
			}
		};

		auto replacement = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf16_char_range{ static_cast<utf16_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(pos + replace_count),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf16_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	constexpr void reserve(size_type new_cap)
	{
		base_.reserve(new_cap);
	}

	[[nodiscard]]
	constexpr auto base() const& noexcept -> const base_type&
	{
		return base_;
	}

	[[nodiscard]]
	constexpr auto base() && noexcept -> base_type&&
	{
		return std::move(base_);
	}

	constexpr void clear()
	{
		base_.clear();
	}

	[[nodiscard]]
	constexpr const char16_t* data() const noexcept
	{
		return base_.data();
	}

	[[nodiscard]]
	constexpr const char16_t* c_str() const noexcept
	{
		return data();
	}

	constexpr operator utf16_string_view() const noexcept
	{
		return as_view();
	}

	[[nodiscard]]
	constexpr equivalent_utf16_string_view as_view() const noexcept
	{
		return equivalent_utf16_string_view::from_code_units_unchecked(equivalent_string_view{ base_ });
	}

	constexpr void push_back(utf16_char ch)
	{
		base_ += equivalent_string_view{ ch.as_view() };
	}

	constexpr void swap(basic_utf16_string& other)
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
	{
		base_.swap(other.base_);
	}

	friend constexpr bool operator==(const basic_utf16_string& lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr bool operator==(const basic_utf16_string& lhs, utf16_string_view rhs) noexcept
	{
		return lhs.base_ == rhs.base();
	}

	friend constexpr bool operator==(utf16_string_view lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base() == rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf16_string& lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf16_string& lhs, utf16_string_view rhs) noexcept
	{
		return lhs.base_ <=> rhs.base();
	}

	friend constexpr auto operator<=>(utf16_string_view lhs, const basic_utf16_string& rhs) noexcept
	{
		return lhs.base() <=> rhs.base_;
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(lhs.base_ + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(lhs.base_ + std::move(rhs.base_));
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + std::move(rhs.base_));
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, utf16_string_view rhs)
	{
		return from_code_units_unchecked(lhs.base_ + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, utf16_string_view rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(utf16_string_view lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(utf16_string_view lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + std::move(rhs.base_));
	}

	friend constexpr basic_utf16_string operator+(const basic_utf16_string& lhs, utf16_char rhs)
	{
		return from_code_units_unchecked(lhs.base_ + base_type{ rhs.as_view(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(basic_utf16_string&& lhs, utf16_char rhs)
	{
		return from_code_units_unchecked(std::move(lhs.base_) + base_type{ rhs.as_view(), lhs.get_allocator() });
	}

	friend constexpr basic_utf16_string operator+(utf16_char lhs, const basic_utf16_string& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.as_view(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf16_string operator+(utf16_char lhs, basic_utf16_string&& rhs)
	{
		return from_code_units_unchecked(base_type{ lhs.as_view(), rhs.get_allocator() } + std::move(rhs.base_));
	}

private:
	using base_class = details::utf16_string_crtp<basic_utf16_string<Allocator>, utf16_string_view>;

	constexpr explicit basic_utf16_string(base_type&& code_units) noexcept
		: base_(std::move(code_units))
	{
	}

	base_type base_;
};

template <typename Allocator>
inline std::ostream& operator<<(std::ostream& os, const basic_utf16_string<Allocator>& value)
{
	return os << value.as_view();
}

}

namespace std
{
	template<typename Allocator>
	struct formatter<unicode_ranges::basic_utf16_string<Allocator>, char> : formatter<unicode_ranges::utf16_string_view, char>
	{
		template<typename FormatContext>
		auto format(const unicode_ranges::basic_utf16_string<Allocator>& value, FormatContext& ctx) const
		{
			return formatter<unicode_ranges::utf16_string_view, char>::format(value.as_view(), ctx);
		}
	};

	template<typename Allocator, typename OtherAllocator>
	struct uses_allocator<unicode_ranges::basic_utf16_string<Allocator>, OtherAllocator> : true_type
	{
	};
}

namespace unicode_ranges
{

namespace literals
{
	template<details::literals::constexpr_utf16_string Str>
	constexpr auto operator ""_utf16_s()
	{
		const auto sv = std::u16string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf16_string_view::from_code_units(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-16");
		}
		return utf16_string(result.value());
	}
}

}

#include "utf8_string.hpp"
#include "transcoding.hpp"

#endif // UTF8_RANGES_UTF16_STRING_HPP
