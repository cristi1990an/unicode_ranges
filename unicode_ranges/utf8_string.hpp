#ifndef UTF8_RANGES_UTF8_STRING_HPP
#define UTF8_RANGES_UTF8_STRING_HPP

#include "utf8_string_view.hpp"

namespace unicode_ranges
{

template <typename Allocator>
class basic_utf8_string : public details::utf8_string_crtp<basic_utf8_string<Allocator>, utf8_string_view>
{
	using equivalent_utf8_string_view = utf8_string_view;
	using equivalent_string_view = std::u8string_view;

 public:
	using base_type = std::basic_string<char8_t, std::char_traits<char8_t>, Allocator>;
	using allocator_type = Allocator;
	using value_type = utf8_char;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	static constexpr auto from_bytes(std::string_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> std::expected<basic_utf8_string, utf8_error>
	{
		if (auto validated = details::copy_validated_utf8_bytes(bytes, alloc); validated) [[likely]]
		{
			return from_base_unchecked(std::move(*validated));
		}

		else
		{
			return std::unexpected(validated.error());
		}
	}

	static constexpr auto from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> std::expected<basic_utf8_string, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		if (auto transcoded = details::transcode_utf16_to_utf8_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_base_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> std::expected<basic_utf8_string, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		if (auto transcoded = details::transcode_unicode_scalars_to_utf8_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_base_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(base_type&& bytes) noexcept
		-> std::expected<basic_utf8_string, utf8_error>
	{
		if (auto validation = details::validate_utf8(equivalent_string_view{ bytes }); !validation) [[unlikely]]
		{
			return std::unexpected(validation.error());
		}

		return from_base_unchecked(std::move(bytes));
	}

	static constexpr auto from_bytes_unchecked(base_type&& bytes) noexcept
		-> basic_utf8_string
	{
		return from_base_unchecked(std::move(bytes));
	}

	static constexpr auto from_bytes_unchecked(std::string_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> basic_utf8_string
	{
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				for (std::size_t index = 0; index != bytes.size(); ++index)
				{
					buffer[index] = static_cast<char8_t>(bytes[index]);
				}

				return bytes.size();
			});

		return from_base_unchecked(std::move(result));
	}

	static constexpr auto from_bytes_unchecked(std::wstring_view bytes, const Allocator& alloc = Allocator()) noexcept
		-> basic_utf8_string
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			base_type result{ alloc };
			result.resize_and_overwrite(bytes.size() * details::encoding_constants::three_code_unit_count,
				[&](char8_t* buffer, std::size_t) noexcept
				{
					std::size_t write_index = 0;
					std::size_t read_index = 0;
					while (read_index < bytes.size())
					{
						const auto first = static_cast<std::uint16_t>(bytes[read_index]);
						const auto count = details::is_utf16_high_surrogate(first)
							? details::encoding_constants::utf16_surrogate_code_unit_count
							: details::encoding_constants::single_code_unit_count;
						const auto scalar = details::decode_valid_utf16_char(
							std::basic_string_view<wchar_t>{ bytes.data() + read_index, count });
						write_index += details::encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
						read_index += count;
					}

					return write_index;
				});

			return from_base_unchecked(std::move(result));
		}

		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size() * details::encoding_constants::max_utf8_code_units,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (wchar_t ch : bytes)
				{
					write_index += details::encode_unicode_scalar_utf8_unchecked(static_cast<std::uint32_t>(ch), buffer + write_index);
				}

				return write_index;
			});

		return from_base_unchecked(std::move(result));
	}

private:
	[[nodiscard]]
	constexpr bool overlaps_base(equivalent_string_view bytes) const noexcept
	{
		if (bytes.empty() || base_.empty())
		{
			return false;
		}

		std::less<const char8_t*> less{};
		const auto* base_begin = base_.data();
		const auto* base_end = base_begin + base_.size();
		const auto* bytes_begin = bytes.data();
		const auto* bytes_end = bytes_begin + bytes.size();
		return less(base_begin, bytes_end) && less(bytes_begin, base_end);
	}

	[[nodiscard]]
	constexpr size_type overlap_offset(equivalent_string_view bytes) const noexcept
	{
		return static_cast<size_type>(bytes.data() - base_.data());
	}

	constexpr basic_utf8_string& append_bytes(equivalent_string_view bytes)
	{
		if (overlaps_base(bytes))
		{
			const auto offset = overlap_offset(bytes);
			base_.append(base_, offset, bytes.size());
		}
		else
		{
			base_.append(bytes);
		}

		return *this;
	}

	constexpr basic_utf8_string& insert_bytes(size_type index, equivalent_string_view bytes)
	{
		if (overlaps_base(bytes))
		{
			const auto offset = overlap_offset(bytes);
			base_.insert(index, base_, offset, bytes.size());
		}
		else
		{
			base_.insert(index, bytes);
		}

		return *this;
	}

	constexpr basic_utf8_string& replace_bytes(size_type pos, size_type count, equivalent_string_view bytes)
	{
		if (overlaps_base(bytes))
		{
			const auto offset = overlap_offset(bytes);
			base_.replace(pos, count, base_, offset, bytes.size());
		}
		else
		{
			base_.replace(pos, count, bytes);
		}

		return *this;
	}

public:

	basic_utf8_string() = default;
	basic_utf8_string(const basic_utf8_string&) = default;
	basic_utf8_string(basic_utf8_string&&) = default;
	basic_utf8_string& operator=(const basic_utf8_string&) = default;
	basic_utf8_string& operator=(basic_utf8_string&&) = default;

	constexpr basic_utf8_string(const Allocator& alloc)
		: base_(alloc)
	{ }

	constexpr basic_utf8_string(const basic_utf8_string& other, const Allocator& alloc)
		: base_(other.base_, alloc)
	{ }

	constexpr basic_utf8_string(basic_utf8_string&& other, const Allocator& alloc)
		noexcept(std::allocator_traits<Allocator>::is_always_equal)
		: base_(std::move(other.base_), alloc)
	{ }

	constexpr basic_utf8_string(utf8_string_view view, const Allocator& alloc = Allocator())
		: base_(view.base(), alloc)
	{}

	constexpr basic_utf8_string(utf16_string_view view, const Allocator& alloc = Allocator());

	constexpr basic_utf8_string(std::size_t count, utf8_char ch, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(count, ch);
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append_range(std::forward<R>(rg));
	}

	constexpr basic_utf8_string(std::initializer_list<utf8_char> ilist, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(ilist);
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string(It it, Sent sent, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(std::move(it), std::move(sent));
	}

	constexpr basic_utf8_string& append_range(views::utf8_view rg);

	constexpr basic_utf8_string& append_range(views::utf16_view rg);

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& append_range(R&& rg)
	{
		base_type appended{ base_.get_allocator() };
		for (utf8_char ch : std::forward<R>(rg))
		{
			appended.append(ch.as_view());
		}
		base_.append(appended);
		return *this;
	}

	constexpr basic_utf8_string& assign_range(views::utf8_view rg);

	constexpr basic_utf8_string& assign_range(views::utf16_view rg);

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& assign_range(R&& rg)
	{
		base_type replacement{ base_.get_allocator() };
		for (utf8_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}
		base_ = std::move(replacement);
		return *this;
	}

	constexpr basic_utf8_string& append(size_type count, utf8_char ch)
	{
		const auto sv = ch.as_view();
		const auto total_size = sv.size() * count;
		const auto old_size = base_.size();

		base_.resize_and_overwrite(old_size + total_size,
			[&](char8_t* buffer, std::size_t)
			{
				buffer = buffer + old_size;
				for (size_type i = 0; i != count; i++)
				{
					std::ranges::copy(sv, buffer);
					buffer += sv.size();
				}

				return total_size;
			});

		return *this;
	}

	constexpr basic_utf8_string& assign(size_type count, utf8_char ch)
	{
		base_.clear();
		return append(count, ch);
	}

	constexpr basic_utf8_string& append(utf8_string_view sv)
	{
		return append_bytes(sv.base());
	}

	constexpr basic_utf8_string& assign(utf8_string_view sv)
	{
		base_.assign(sv.base());
		return *this;
	}

	constexpr basic_utf8_string& assign(utf8_char ch)
	{
		base_.assign(ch.as_view());
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string& append(It it, Sent sent)
	{
		return append_range(std::ranges::subrange(std::move(it), std::move(sent)));
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string& assign(It it, Sent sent)
	{
		base_.clear();
		return append(std::move(it), std::move(sent));
	}

	constexpr basic_utf8_string& append(std::initializer_list<utf8_char> ilist)
	{
		return append_range(ilist);
	}

	constexpr basic_utf8_string& assign(std::initializer_list<utf8_char> ilist)
	{
		return assign_range(ilist);
	}

	constexpr basic_utf8_string& operator=(utf8_string_view sv)
	{
		return assign(sv);
	}

	constexpr basic_utf8_string& operator=(utf8_char ch)
	{
		return assign(ch);
	}

	constexpr basic_utf8_string& operator=(std::initializer_list<utf8_char> ilist)
	{
		return assign(ilist);
	}

	constexpr basic_utf8_string& operator+=(utf8_string_view sv)
	{
		return append(sv);
	}

	constexpr basic_utf8_string& operator+=(utf16_string_view sv);

	constexpr basic_utf8_string& operator+=(utf8_char ch)
	{
		push_back(ch);
		return *this;
	}

	constexpr basic_utf8_string& operator+=(utf16_char ch)
	{
		push_back(static_cast<utf8_char>(ch));
		return *this;
	}

	constexpr basic_utf8_string& operator+=(std::initializer_list<utf8_char> ilist)
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

	constexpr basic_utf8_string& insert(size_type index, utf8_string_view sv)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
		}

		return insert_bytes(index, sv.base());
	}

	constexpr basic_utf8_string& insert(size_type index, utf8_char ch)
	{
		return insert(index, ch.as_utf8_view());
	}

	constexpr basic_utf8_string& insert(size_type index, size_type count, utf8_char ch)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
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

	constexpr basic_utf8_string& insert_range(size_type index, views::utf8_view rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
		}

		return insert_bytes(index, rg.base());
	}

	constexpr basic_utf8_string& insert_range(size_type index, views::utf16_view rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
		}

		base_type inserted{ base_.get_allocator() };
		inserted.reserve(rg.base().size() * 3u);
		for (utf16_char ch : rg)
		{
			std::array<char8_t, 4> encoded{};
			const auto encoded_count = ch.encode_utf8<char8_t>(encoded.begin());
			inserted.append(encoded.data(), encoded.data() + encoded_count);
		}

		base_.insert(index, inserted);
		return *this;
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& insert_range(size_type index, R&& rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-8 character boundary");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf8_char_range
		{
			utf8_char ch;

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
					return encoded_utf8_char_range{ static_cast<utf8_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.insert_range(base_.begin() + static_cast<difference_type>(index), inserted);
#else
		const utf8_string inserted(std::from_range, std::forward<R>(rg));
		base_.insert(index, inserted.base());
#endif
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf8_string& insert(size_type index, It first, Sent last)
	{
		return insert_range(index, std::ranges::subrange(std::move(first), std::move(last)));
	}

	constexpr basic_utf8_string& insert(size_type index, std::initializer_list<utf8_char> ilist)
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
		const auto bytes_to_remove = removed.code_unit_count();
		const auto where_idx = base_.size() - bytes_to_remove;
		base_.erase(where_idx, bytes_to_remove);
		return removed;
	}

	constexpr basic_utf8_string& erase(size_type index, size_type count = npos)
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
			throw std::out_of_range("erase range must be a valid UTF-8 substring");
		}

		base_.erase(index, erase_count);
		return *this;
	}

	constexpr basic_utf8_string& replace(size_type pos, size_type count, utf8_string_view other)
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
			throw std::out_of_range("replace range must be a valid UTF-8 substring");
		}

		return replace_bytes(pos, replace_count, other.base());
	}

	constexpr basic_utf8_string& replace(size_type pos, size_type count, utf8_char other)
	{
		return replace(pos, count, other.as_utf8_view());
	}

	constexpr basic_utf8_string& replace(size_type pos, utf8_string_view other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_bytes(pos, replace_count, other.base());
	}

	constexpr basic_utf8_string& replace(size_type pos, utf8_char other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		base_.replace(pos, replace_count, other.as_view());
		return *this;
	}

	constexpr basic_utf8_string& replace_with_range(size_type pos, size_type count, views::utf8_view rg)
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
			throw std::out_of_range("replace range must be a valid UTF-8 substring");
		}

		return replace_bytes(pos, replace_count, rg.base());
	}

	constexpr basic_utf8_string& replace_with_range(size_type pos, size_type count, views::utf16_view rg)
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
			throw std::out_of_range("replace range must be a valid UTF-8 substring");
		}

		base_type replacement{ base_.get_allocator() };
		replacement.reserve(rg.base().size() * 3u);
		for (utf16_char ch : rg)
		{
			std::array<char8_t, 4> encoded{};
			const auto encoded_count = ch.encode_utf8<char8_t>(encoded.begin());
			replacement.append(encoded.data(), encoded.data() + encoded_count);
		}

		base_.replace(pos, replace_count, replacement);
		return *this;
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& replace_with_range(size_type pos, size_type count, R&& rg)
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
			throw std::out_of_range("replace range must be a valid UTF-8 substring");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf8_char_range
		{
			utf8_char ch;

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
					return encoded_utf8_char_range{ static_cast<utf8_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(end),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf8_char ch : std::forward<R>(rg))
		{
			replacement.append(ch.as_view());
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& replace_with_range(size_type pos, views::utf8_view rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_with_range(pos, replace_count, rg);
	}

	constexpr basic_utf8_string& replace_with_range(size_type pos, views::utf16_view rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_with_range(pos, replace_count, rg);
	}

	template <details::container_compatible_range<utf8_char> R>
	constexpr basic_utf8_string& replace_with_range(size_type pos, R&& rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-8 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf8_char_range
		{
			utf8_char ch;

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
					return encoded_utf8_char_range{ static_cast<utf8_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(pos + replace_count),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf8_char ch : std::forward<R>(rg))
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
	constexpr const char8_t* data() const noexcept
	{
		return base_.data();
	}

	[[nodiscard]]
	constexpr const char8_t* c_str() const noexcept
	{
		return data();
	}

	constexpr operator utf8_string_view() const noexcept
	{
		return as_view();
	}

	[[nodiscard]]
	constexpr equivalent_utf8_string_view as_view() const noexcept
	{
		return equivalent_utf8_string_view::from_bytes_unchecked(equivalent_string_view{ base_ });
	}

	constexpr void push_back(utf8_char ch)
	{
		std::array<char8_t, 4> tmp{};
		const auto size = ch.encode_utf8<char8_t>(tmp.begin());
		base_ += equivalent_string_view{ tmp.data(), size };
	}

	constexpr void swap(basic_utf8_string& other)
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
	{
		base_.swap(other.base_);
	}

	friend constexpr bool operator==(const basic_utf8_string& lhs, const basic_utf8_string& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr bool operator==(const basic_utf8_string& lhs, utf8_string_view rhs) noexcept
	{
		return lhs.base_ == rhs.base();
	}

	friend constexpr bool operator==(utf8_string_view lhs, const basic_utf8_string& rhs) noexcept
	{
		return lhs.base() == rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf8_string& lhs, const basic_utf8_string& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf8_string& lhs, utf8_string_view rhs) noexcept
	{
		return lhs.base_ <=> rhs.base();
	}

	friend constexpr auto operator<=>(utf8_string_view lhs, const basic_utf8_string& rhs) noexcept
	{
		return lhs.base() <=> rhs.base_;
	}

	friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs, const basic_utf8_string& rhs)
	{
		return from_base_unchecked(lhs.base_ + rhs.base_);
	}

	friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs, const basic_utf8_string& rhs)
	{
		return from_base_unchecked(std::move(lhs.base_) + rhs.base_);
	}

	friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs, basic_utf8_string&& rhs)
	{
		return from_base_unchecked(lhs.base_ + std::move(rhs.base_));
	}

	friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs, basic_utf8_string&& rhs)
	{
		return from_base_unchecked(std::move(lhs.base_) + std::move(rhs.base_));
	}

	friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs, utf8_string_view rhs)
	{
		return from_base_unchecked(lhs.base_ + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs, utf8_string_view rhs)
	{
		return from_base_unchecked(std::move(lhs.base_) + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf8_string operator+(utf8_string_view lhs, const basic_utf8_string& rhs)
	{
		return from_base_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf8_string operator+(utf8_string_view lhs, basic_utf8_string&& rhs)
	{
		return from_base_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + std::move(rhs.base_));
	}

	friend constexpr basic_utf8_string operator+(const basic_utf8_string& lhs, utf8_char rhs)
	{
		return from_base_unchecked(lhs.base_ + base_type{ rhs.as_view(), lhs.get_allocator() });
	}

	friend constexpr basic_utf8_string operator+(basic_utf8_string&& lhs, utf8_char rhs)
	{
		return from_base_unchecked(std::move(lhs.base_) + base_type{ rhs.as_view(), lhs.get_allocator() });
	}

	friend constexpr basic_utf8_string operator+(utf8_char lhs, const basic_utf8_string& rhs)
	{
		return from_base_unchecked(base_type{ lhs.as_view(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf8_string operator+(utf8_char lhs, basic_utf8_string&& rhs)
	{
		return from_base_unchecked(base_type{ lhs.as_view(), rhs.get_allocator() } + std::move(rhs.base_));
	}

private:
	using base_class = details::utf8_string_crtp<basic_utf8_string<Allocator>, utf8_string_view>;

	static constexpr basic_utf8_string from_base_unchecked(base_type bytes) noexcept
	{
		basic_utf8_string result;
		result.base_ = std::move(bytes);
		return result;
	}

	base_type base_;
};

template <typename Allocator>
inline std::ostream& operator<<(std::ostream& os, const basic_utf8_string<Allocator>& value)
{
	return os << value.as_view();
}

}

namespace std
{
	template<typename Allocator>
	struct formatter<unicode_ranges::basic_utf8_string<Allocator>, char> : formatter<unicode_ranges::utf8_string_view, char>
	{
		template<typename FormatContext>
		auto format(const unicode_ranges::basic_utf8_string<Allocator>& value, FormatContext& ctx) const
		{
			return formatter<unicode_ranges::utf8_string_view, char>::format(value.as_view(), ctx);
		}
	};

	template<typename Allocator, typename OtherAllocator>
	struct uses_allocator<unicode_ranges::basic_utf8_string<Allocator>, OtherAllocator> : true_type
	{
	};
}

namespace unicode_ranges
{

namespace literals
{
	template<details::literals::constexpr_utf8_string Str>
	constexpr auto operator ""_utf8_s()
	{
		const auto sv = std::u8string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf8_string_view::from_bytes(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-8");
		}
		return utf8_string(result.value());
	}
}

}

#include "utf16_string.hpp"
#include "transcoding.hpp"

#endif // UTF8_RANGES_UTF8_STRING_HPP
