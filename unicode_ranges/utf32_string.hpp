#ifndef UTF8_RANGES_UTF32_STRING_HPP
#define UTF8_RANGES_UTF32_STRING_HPP

#include <algorithm>

#include "utf32_string_view.hpp"
#include "encoding.hpp"

namespace unicode_ranges
{

template <typename Allocator>
class basic_utf32_string : public details::utf32_string_crtp<basic_utf32_string<Allocator>, utf32_string_view>
{
	using crtp = details::utf32_string_crtp<basic_utf32_string<Allocator>, utf32_string_view>;
	using equivalent_utf32_string_view = utf32_string_view;
	using equivalent_string_view = std::u32string_view;

public:
	using base_type = std::basic_string<char32_t, std::char_traits<char32_t>, Allocator>;
	using allocator_type = Allocator;
	using value_type = utf32_char;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	static constexpr auto from_bytes(std::string_view bytes, const Allocator& alloc = Allocator())
		-> std::expected<basic_utf32_string, utf8_error>
	{
		if (auto transcoded = details::transcode_utf8_to_utf32_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_code_points_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator())
		-> std::expected<basic_utf32_string, utf16_error>
		requires (sizeof(wchar_t) == 2)
	{
		if (auto transcoded = details::transcode_utf16_to_utf32_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_code_points_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(std::wstring_view bytes, const Allocator& alloc = Allocator())
		-> std::expected<basic_utf32_string, unicode_scalar_error>
		requires (sizeof(wchar_t) == 4)
	{
		if (auto transcoded = details::transcode_unicode_scalars_to_utf32_checked(bytes, alloc); transcoded) [[likely]]
		{
			return from_code_points_unchecked(std::move(*transcoded));
		}

		else
		{
			return std::unexpected(transcoded.error());
		}
	}

	static constexpr auto from_bytes(base_type&& bytes) noexcept
		-> std::expected<basic_utf32_string, utf32_error>
	{
		if (auto validation = details::validate_utf32(equivalent_string_view{ bytes }); !validation) [[unlikely]]
		{
			return std::unexpected(validation.error());
		}

		return from_code_points_unchecked(std::move(bytes));
	}

	static constexpr basic_utf32_string from_code_points_unchecked(base_type code_points) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf32(equivalent_string_view{ code_points }).has_value());
		return basic_utf32_string{ std::move(code_points) };
	}

	static constexpr basic_utf32_string from_code_points_unchecked(base_type code_points, const Allocator& alloc)
	{
		return from_code_points_unchecked(equivalent_string_view{ code_points }, alloc);
	}

	static constexpr basic_utf32_string from_code_points_unchecked(
		equivalent_string_view code_points,
		const Allocator& alloc = Allocator())
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf32(code_points).has_value());

		basic_utf32_string result;
		result.base_ = base_type{ code_points, alloc };
		return result;
	}

	static constexpr basic_utf32_string from_bytes_unchecked(base_type&& bytes) noexcept
	{
		return from_code_points_unchecked(std::move(bytes));
	}

	static constexpr basic_utf32_string from_code_points_lossy(
		equivalent_string_view code_points,
		const Allocator& alloc = Allocator())
	{
		return from_code_points_unchecked(details::copy_lossy_utf32_code_points(code_points, alloc));
	}

	static constexpr basic_utf32_string from_code_points_lossy(base_type&& code_points) noexcept
	{
		details::repair_utf32_code_points_inplace(code_points);
		return from_code_points_unchecked(std::move(code_points));
	}

	static constexpr auto from_bytes_unchecked(std::string_view bytes, const Allocator& alloc = Allocator())
		-> basic_utf32_string
	{
		UTF8_RANGES_DEBUG_ASSERT(details::validate_utf8(bytes).has_value());

		base_type utf32_code_points{ alloc };
		utf32_code_points.resize_and_overwrite(bytes.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(
						std::basic_string_view<char>{ bytes.data() + read_index, count });
					write_index += details::encode_unicode_scalar_utf32_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

		return from_code_points_unchecked(std::move(utf32_code_points));
	}

	static constexpr auto from_bytes_unchecked(std::wstring_view bytes, const Allocator& alloc = Allocator())
		-> basic_utf32_string
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			UTF8_RANGES_DEBUG_ASSERT(details::validate_utf16(bytes).has_value());

			base_type result{ alloc };
			result.resize_and_overwrite(bytes.size(),
				[&](char32_t* buffer, std::size_t) noexcept
				{
					std::size_t write_index = 0;
					std::size_t read_index = 0;
					while (read_index < bytes.size())
					{
						const auto remaining = std::wstring_view{ bytes.data() + read_index, bytes.size() - read_index };
						const auto ascii_run = details::ascii_prefix_length(remaining);
						if (ascii_run != 0)
						{
							for (std::size_t i = 0; i != ascii_run; ++i)
							{
								buffer[write_index + i] = static_cast<char32_t>(remaining[i]);
							}

							write_index += ascii_run;
							read_index += ascii_run;
							continue;
						}

						const auto first = static_cast<std::uint16_t>(bytes[read_index]);
						const auto count = details::is_utf16_high_surrogate(first)
							? details::encoding_constants::utf16_surrogate_code_unit_count
							: details::encoding_constants::single_code_unit_count;
						buffer[write_index++] = static_cast<char32_t>(
							details::decode_valid_utf16_char(bytes.data() + read_index, count));
						read_index += count;
					}

					return write_index;
				});

			return from_code_points_unchecked(std::move(result));
		}
		else
		{
			UTF8_RANGES_DEBUG_ASSERT(details::validate_unicode_scalars(bytes).has_value());

			base_type utf32_code_points{ alloc };
			utf32_code_points.resize_and_overwrite(bytes.size() * details::encoding_constants::utf32_surrogate_code_unit_count,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					std::size_t write_index = 0;
					for (wchar_t ch : bytes)
					{
						write_index += details::encode_unicode_scalar_utf32_unchecked(static_cast<std::uint32_t>(ch), buffer + write_index);
					}

					return write_index;
				});

			return from_code_points_unchecked(std::move(utf32_code_points));
		}
	}

	template <typename Decoder>
	static constexpr auto from_encoded(
		std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
		Decoder& decoder,
		const Allocator& alloc = Allocator())
		-> from_encoded_result<Decoder, basic_utf32_string>
	{
		base_type output{ alloc };
		details::container_append_writer<char32_t, base_type> code_unit_writer{ output };
		details::utf32_scalar_writer<decltype(code_unit_writer)> scalar_writer{ code_unit_writer };

		if constexpr (std::same_as<typename decoder_traits<Decoder>::decode_error, void>)
		{
			details::decode_whole_input_to_utf32(decoder, input, code_unit_writer, scalar_writer);
			details::validate_utf_result(output);
			return from_code_points_unchecked(std::move(output));
		}
		else
		{
			auto result = details::decode_whole_input_to_utf32(decoder, input, code_unit_writer, scalar_writer);
			if (!result) [[unlikely]]
			{
				return std::unexpected(result.error());
			}

			details::validate_utf_result(output);
			return from_code_points_unchecked(std::move(output));
		}
	}

	template <typename Decoder>
		requires decoder_traits<Decoder>::allow_implicit_construction_requested
	static constexpr auto from_encoded(
		std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
		const Allocator& alloc = Allocator())
		-> from_encoded_result<Decoder, basic_utf32_string>
	{
		static_assert(std::default_initializable<Decoder>,
			"implicitly constructed decoders must be default constructible");

		Decoder decoder{};
		return from_encoded(input, decoder, alloc);
	}

private:
	[[nodiscard]]
	constexpr bool overlaps_base(equivalent_string_view code_points) const noexcept
	{
		if (code_points.empty() || base_.empty())
		{
			return false;
		}

		std::less<const char32_t*> less{};
		const auto* base_begin = base_.data();
		const auto* base_end = base_begin + base_.size();
		const auto* source_begin = code_points.data();
		const auto* source_end = source_begin + code_points.size();
		return less(base_begin, source_end) && less(source_begin, base_end);
	}

	[[nodiscard]]
	constexpr size_type overlap_offset(equivalent_string_view code_points) const noexcept
	{
		return static_cast<size_type>(code_points.data() - base_.data());
	}

	constexpr basic_utf32_string& append_code_points(equivalent_string_view code_points)
	{
		if (overlaps_base(code_points))
		{
			const auto offset = overlap_offset(code_points);
			base_.append(base_, offset, code_points.size());
		}
		else
		{
			base_.append(code_points);
		}

		return *this;
	}

	constexpr basic_utf32_string& insert_code_points(size_type index, equivalent_string_view code_points)
	{
		if (overlaps_base(code_points))
		{
			const auto offset = overlap_offset(code_points);
			base_.insert(index, base_, offset, code_points.size());
		}
		else
		{
			base_.insert(index, code_points);
		}

		return *this;
	}

	constexpr basic_utf32_string& replace_code_points(size_type pos, size_type count, equivalent_string_view code_points)
	{
		if (overlaps_base(code_points))
		{
			const auto offset = overlap_offset(code_points);
			base_.replace(pos, count, base_, offset, code_points.size());
		}
		else
		{
			base_.replace(pos, count, code_points);
		}

		return *this;
	}

	template <typename ResultAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<ResultAllocator> replace_code_points_copy(
		equivalent_string_view needle,
		equivalent_string_view replacement,
		size_type count,
		const ResultAllocator& alloc) const
	{
		return basic_utf32_string<ResultAllocator>::from_code_points_unchecked(details::replace_utf32_code_points_copy(
			equivalent_string_view{ base_ },
			needle,
			replacement,
			count,
			alloc));
	}

	constexpr basic_utf32_string& replace_code_points_in_place(
		equivalent_string_view needle,
		equivalent_string_view replacement,
		size_type count)
	{
		if (needle.empty() || count == 0)
		{
			return *this;
		}

		if (needle == replacement) [[unlikely]]
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

		if (needle.size() == replacement.size())
		{
			if consteval
			{
				size_type search_pos = 0;
				size_type replacements = 0;
				const auto replacement_size = replacement.size();
				while (replacements != count)
				{
					const auto match = details::find_utf32_exact(equivalent_string_view{ base_ }, needle, search_pos);
					if (match == equivalent_string_view::npos)
					{
						break;
					}

					std::ranges::copy(replacement, base_.begin() + static_cast<difference_type>(match));
					search_pos = match + replacement_size;
					++replacements;
				}

				return *this;
			}

			if (needle.size() == 1u)
			{
				size_type search_pos = 0;
				size_type replacements = 0;
				while (replacements != count)
				{
					const auto match = base_.find(needle.front(), search_pos);
					if (match == base_type::npos)
					{
						break;
					}

					base_[match] = replacement.front();
					search_pos = match + 1u;
					++replacements;
				}

				return *this;
			}

			const details::utf32_runtime_exact_searcher searcher{ needle };
			size_type search_pos = 0;
			size_type replacements = 0;
			const auto replacement_size = replacement.size();
			while (replacements != count)
			{
				const auto match = searcher.find(equivalent_string_view{ base_ }, search_pos);
				if (match == equivalent_string_view::npos)
				{
					break;
				}

				std::char_traits<char32_t>::copy(base_.data() + match, replacement.data(), replacement_size);
				search_pos = match + replacement_size;
				++replacements;
			}

			return *this;
		}

		if consteval
		{
			size_type search_pos = 0;
			size_type replacements = 0;
			while (replacements != count)
			{
				const auto match = details::find_utf32_exact(equivalent_string_view{ base_ }, needle, search_pos);
				if (match == equivalent_string_view::npos)
				{
					break;
				}

				replace_code_points(match, needle.size(), replacement);
				search_pos = match + replacement.size();
				++replacements;
			}

			return *this;
		}

		base_type rebuilt = details::replace_utf32_code_points_copy(
			equivalent_string_view{ base_ },
			needle,
			replacement,
			count,
			base_.get_allocator());
		base_.swap(rebuilt);
		return *this;
	}

	constexpr basic_utf32_string& reverse_code_points_unchecked(size_type pos, size_type count) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(pos <= size());
		UTF8_RANGES_DEBUG_ASSERT(count <= size() - pos);
		UTF8_RANGES_DEBUG_ASSERT(this->is_char_boundary(pos));
		UTF8_RANGES_DEBUG_ASSERT(this->is_char_boundary(pos + count));

		std::reverse(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(pos + count));
		return *this;
	}

	constexpr basic_utf32_string& reverse_grapheme_code_points_unchecked(size_type pos, size_type count) noexcept
	{
		UTF8_RANGES_DEBUG_ASSERT(pos <= size());
		UTF8_RANGES_DEBUG_ASSERT(count <= size() - pos);
		UTF8_RANGES_DEBUG_ASSERT(this->is_grapheme_boundary(pos));
		UTF8_RANGES_DEBUG_ASSERT(this->is_grapheme_boundary(pos + count));

		const auto end = pos + count;
		const auto code_points = equivalent_string_view{ base_ };
		for (size_type index = pos; index < end; )
		{
			const auto next = details::next_grapheme_boundary(code_points, index);
			std::reverse(
				base_.begin() + static_cast<difference_type>(index),
				base_.begin() + static_cast<difference_type>(next));
			index = next;
		}

		std::reverse(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(end));
		return *this;
	}

public:

	basic_utf32_string() = default;
	basic_utf32_string(const basic_utf32_string&) = default;
	basic_utf32_string(basic_utf32_string&&) = default;
	basic_utf32_string& operator=(const basic_utf32_string&) = default;
	basic_utf32_string& operator=(basic_utf32_string&&) = default;

	constexpr basic_utf32_string(const Allocator& alloc)
		: base_(alloc)
	{ }

	constexpr basic_utf32_string(const basic_utf32_string& other, const Allocator& alloc)
		: base_(other.base_, alloc)
	{ }

	constexpr basic_utf32_string(basic_utf32_string&& other, const Allocator& alloc)
		noexcept(std::allocator_traits<Allocator>::is_always_equal)
		: base_(std::move(other.base_), alloc)
	{ }

	constexpr basic_utf32_string(utf32_string_view view, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		const auto code_points = view.base();
		base_.resize_and_overwrite(code_points.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, code_points.data(), code_points.size());
				return code_points.size();
			});
	}

	constexpr basic_utf32_string(utf8_string_view view, const Allocator& alloc = Allocator());
	constexpr basic_utf32_string(utf16_string_view view, const Allocator& alloc = Allocator());

	constexpr basic_utf32_string(std::size_t count, utf32_char ch, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(count, ch);
	}

	template <details::container_compatible_range<utf32_char> R>
	constexpr basic_utf32_string(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append_range(std::forward<R>(rg));
	}

	constexpr basic_utf32_string(std::initializer_list<utf32_char> ilist, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(ilist);
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf32_string(It it, Sent sent, const Allocator& alloc = Allocator())
		: base_(alloc)
	{
		append(std::move(it), std::move(sent));
	}

	constexpr basic_utf32_string& append_range(views::utf32_view rg);

	constexpr basic_utf32_string& append_range(views::utf8_view rg);

	constexpr basic_utf32_string& append_range(views::utf16_view rg);

	template <details::container_compatible_range<utf32_char> R>
	constexpr basic_utf32_string& append_range(R&& rg)
	{
		if constexpr (details::contiguous_sized_range_of<R, utf32_char>)
		{
			const auto* chars = std::ranges::data(rg);
			const auto count = static_cast<size_type>(std::ranges::size(rg));
			const auto appended_size = static_cast<size_type>(
				details::utf32_char_sequence_code_unit_count(chars, count));
			const auto old_size = base_.size();
			base_.resize_and_overwrite(old_size + appended_size,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					details::copy_utf32_char_sequence(chars, count, buffer + old_size);
					return old_size + appended_size;
				});
			return *this;
		}

		if constexpr (std::ranges::sized_range<R>)
		{
			const auto upper_bound = static_cast<size_type>(std::ranges::size(rg))
				* details::encoding_constants::utf32_surrogate_code_unit_count;
			const auto old_size = base_.size();
			base_.resize_and_overwrite(old_size + upper_bound,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					auto* out = buffer + old_size;
					for (utf32_char ch : rg)
					{
						const auto sv = details::utf32_char_view(ch);
						std::char_traits<char32_t>::copy(out, sv.data(), sv.size());
						out += sv.size();
					}

					return old_size + static_cast<size_type>(out - (buffer + old_size));
				});
			return *this;
		}

		if constexpr (std::ranges::forward_range<R>)
		{
			size_type appended_size = 0;
			for (utf32_char ch : rg)
			{
				appended_size += details::utf32_char_view(ch).size();
			}

			const auto old_size = base_.size();
			base_.resize_and_overwrite(old_size + appended_size,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					auto* out = buffer + old_size;
					for (utf32_char ch : rg)
					{
						const auto sv = details::utf32_char_view(ch);
						std::ranges::copy(sv, out);
						out += sv.size();
					}

					return old_size + appended_size;
				});
			return *this;
		}

		for (utf32_char ch : rg)
		{
			base_.append(details::utf32_char_view(ch));
		}

		return *this;
	}

	constexpr basic_utf32_string& assign_range(views::utf32_view rg);

	constexpr basic_utf32_string& assign_range(views::utf8_view rg);

	constexpr basic_utf32_string& assign_range(views::utf16_view rg);

	template <details::container_compatible_range<utf32_char> R>
	constexpr basic_utf32_string& assign_range(R&& rg)
	{
		if constexpr (details::contiguous_sized_range_of<R, utf32_char>)
		{
			const auto* chars = std::ranges::data(rg);
			const auto count = static_cast<size_type>(std::ranges::size(rg));
			const auto replacement_size = static_cast<size_type>(
				details::utf32_char_sequence_code_unit_count(chars, count));
			base_.resize_and_overwrite(replacement_size,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					details::copy_utf32_char_sequence(chars, count, buffer);
					return replacement_size;
				});
			return *this;
		}

		base_type replacement{ base_.get_allocator() };
		if constexpr (std::ranges::sized_range<R>)
		{
			const auto upper_bound = static_cast<size_type>(std::ranges::size(rg))
				* details::encoding_constants::utf32_surrogate_code_unit_count;
			replacement.resize_and_overwrite(upper_bound,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					auto* out = buffer;
					for (utf32_char ch : rg)
					{
						const auto sv = details::utf32_char_view(ch);
						std::char_traits<char32_t>::copy(out, sv.data(), sv.size());
						out += sv.size();
					}

					return static_cast<size_type>(out - buffer);
				});
			base_ = std::move(replacement);
			return *this;
		}

		if constexpr (std::ranges::forward_range<R>)
		{
			size_type replacement_size = 0;
			for (utf32_char ch : rg)
			{
				replacement_size += details::utf32_char_view(ch).size();
			}

			replacement.resize_and_overwrite(replacement_size,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					auto* out = buffer;
					for (utf32_char ch : rg)
					{
						const auto sv = details::utf32_char_view(ch);
						std::ranges::copy(sv, out);
						out += sv.size();
					}

					return replacement_size;
				});
			base_ = std::move(replacement);
			return *this;
		}

		for (utf32_char ch : rg)
		{
			replacement.append(details::utf32_char_view(ch));
		}
		base_ = std::move(replacement);
		return *this;
	}

	constexpr basic_utf32_string& append(size_type count, utf32_char ch)
	{
		const auto sv = details::utf32_char_view(ch);
		const auto total_size = sv.size() * count;
		const auto old_size = base_.size();

		base_.resize_and_overwrite(old_size + total_size,
			[&](char32_t* buffer, std::size_t)
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

	constexpr basic_utf32_string& assign(size_type count, utf32_char ch)
	{
		base_.clear();
		return append(count, ch);
	}

	constexpr basic_utf32_string& append(utf32_string_view sv)
	{
		return append_code_points(sv.base());
	}

	constexpr basic_utf32_string& assign(utf32_string_view sv)
	{
		base_.assign(sv.base());
		return *this;
	}

	constexpr basic_utf32_string& assign(utf32_char ch)
	{
		base_.assign(details::utf32_char_view(ch));
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf32_string& append(It it, Sent sent)
	{
		return append_range(std::ranges::subrange(std::move(it), std::move(sent)));
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf32_string& assign(It it, Sent sent)
	{
		base_.clear();
		return append(std::move(it), std::move(sent));
	}

	constexpr basic_utf32_string& append(std::initializer_list<utf32_char> ilist)
	{
		return append_range(ilist);
	}

	template <
		typename Encoder,
		typename OutputAllocator = std::allocator<typename encoder_traits<Encoder>::code_unit_type>>
	[[nodiscard]]
	constexpr auto to_encoded(Encoder& encoder, const OutputAllocator& alloc = OutputAllocator()) const
		-> to_encoded_result<Encoder, OutputAllocator>
	{
		using output_string = std::basic_string<
			typename encoder_traits<Encoder>::code_unit_type,
			std::char_traits<typename encoder_traits<Encoder>::code_unit_type>,
			OutputAllocator>;

		output_string output{ alloc };
		details::container_append_writer<typename encoder_traits<Encoder>::code_unit_type, output_string> writer{ output };
		if constexpr (std::same_as<typename encoder_traits<Encoder>::encode_error, void>)
		{
			details::encode_whole_input(encoder, this->as_view(), writer);
			return output;
		}
		else
		{
			auto result = details::encode_whole_input(encoder, this->as_view(), writer);
			if (!result) [[unlikely]]
			{
				return std::unexpected(result.error());
			}

			return output;
		}
	}

	template <
		typename Encoder,
		typename OutputAllocator = std::allocator<typename encoder_traits<Encoder>::code_unit_type>>
		requires encoder_traits<Encoder>::allow_implicit_construction_requested
	[[nodiscard]]
	constexpr auto to_encoded(const OutputAllocator& alloc = OutputAllocator()) const
		-> to_encoded_result<Encoder, OutputAllocator>
	{
		static_assert(std::default_initializable<Encoder>,
			"implicitly constructed encoders must be default constructible");

		Encoder encoder{};
		return to_encoded(encoder, alloc);
	}

	template <typename Encoder, typename Out>
	constexpr auto encode_to(Out&& out, Encoder& encoder) const
		-> std::expected<void, encode_to_error<Encoder>>
		requires std::ranges::range<Out>
			&& std::ranges::output_range<Out, typename encoder_traits<Encoder>::code_unit_type>
	{
		using unit_type = typename encoder_traits<Encoder>::code_unit_type;
		auto first = std::ranges::begin(out);
		auto last = std::ranges::end(out);
		details::bounded_output_state<unit_type, decltype(first), decltype(last)> state{
			.current = std::move(first),
			.end = std::move(last)
		};
		details::bounded_output_writer<unit_type, decltype(state.current), decltype(state.end)> writer{ state };

		if constexpr (std::same_as<typename encoder_traits<Encoder>::encode_error, void>)
		{
			details::encode_whole_input(encoder, this->as_view(), writer);
			if (writer.overflowed()) [[unlikely]]
			{
				return std::unexpected(encode_to_error<Encoder>::overflow());
			}

			return {};
		}
		else
		{
			auto result = details::encode_whole_input(encoder, this->as_view(), writer);
			if (writer.overflowed()) [[unlikely]]
			{
				return std::unexpected(encode_to_error<Encoder>::overflow());
			}

			if (!result) [[unlikely]]
			{
				return std::unexpected(encode_to_error<Encoder>::encoding(result.error()));
			}

			return {};
		}
	}

	template <typename Encoder, typename Out>
		requires encoder_traits<Encoder>::allow_implicit_construction_requested
			&& std::ranges::range<Out>
			&& std::ranges::output_range<Out, typename encoder_traits<Encoder>::code_unit_type>
	constexpr auto encode_to(Out&& out) const
		-> std::expected<void, encode_to_error<Encoder>>
	{
		static_assert(std::default_initializable<Encoder>,
			"implicitly constructed encoders must be default constructible");

		Encoder encoder{};
		return encode_to(std::forward<Out>(out), encoder);
	}

	template <typename Encoder, typename Container>
	constexpr auto encode_append_to(Container& container, Encoder& encoder) const
		-> std::conditional_t<
			std::same_as<typename encoder_traits<Encoder>::encode_error, void>,
			void,
			std::expected<void, typename encoder_traits<Encoder>::encode_error>>
		requires details::sequence_like_append_container<Container, typename encoder_traits<Encoder>::code_unit_type>
	{
		details::container_append_writer<typename encoder_traits<Encoder>::code_unit_type, Container> writer{ container };
		if constexpr (std::same_as<typename encoder_traits<Encoder>::encode_error, void>)
		{
			details::encode_whole_input(encoder, this->as_view(), writer);
		}
		else
		{
			return details::encode_whole_input(encoder, this->as_view(), writer);
		}
	}

	template <typename Encoder, typename Container>
		requires encoder_traits<Encoder>::allow_implicit_construction_requested
			&& details::sequence_like_append_container<Container, typename encoder_traits<Encoder>::code_unit_type>
	constexpr auto encode_append_to(Container& container) const
		-> std::conditional_t<
			std::same_as<typename encoder_traits<Encoder>::encode_error, void>,
			void,
			std::expected<void, typename encoder_traits<Encoder>::encode_error>>
	{
		static_assert(std::default_initializable<Encoder>,
			"implicitly constructed encoders must be default constructible");

		Encoder encoder{};
		if constexpr (std::same_as<typename encoder_traits<Encoder>::encode_error, void>)
		{
			encode_append_to(container, encoder);
		}
		else
		{
			return encode_append_to(container, encoder);
		}
	}

	constexpr basic_utf32_string& assign(std::initializer_list<utf32_char> ilist)
	{
		return assign_range(ilist);
	}

	constexpr basic_utf32_string& operator=(utf32_string_view sv)
	{
		return assign(sv);
	}

	constexpr basic_utf32_string& operator=(utf32_char ch)
	{
		return assign(ch);
	}

	constexpr basic_utf32_string& operator=(std::initializer_list<utf32_char> ilist)
	{
		return assign(ilist);
	}

	constexpr basic_utf32_string& operator+=(utf32_string_view sv)
	{
		return append(sv);
	}

	constexpr basic_utf32_string& operator+=(utf8_string_view sv);

	constexpr basic_utf32_string& operator+=(utf16_string_view sv);

	constexpr basic_utf32_string& operator+=(utf32_char ch)
	{
		push_back(ch);
		return *this;
	}

	constexpr basic_utf32_string& operator+=(utf8_char ch)
	{
		push_back(static_cast<utf32_char>(ch));
		return *this;
	}

	constexpr basic_utf32_string& operator+=(utf16_char ch)
	{
		push_back(static_cast<utf32_char>(ch));
		return *this;
	}

	constexpr basic_utf32_string& operator+=(std::initializer_list<utf32_char> ilist)
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

	constexpr basic_utf32_string& insert(size_type index, utf32_string_view sv)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-32 character boundary");
		}

		return insert_code_points(index, sv.base());
	}

	constexpr basic_utf32_string& insert(size_type index, utf32_char ch)
	{
		return insert(index, equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(ch)));
	}

	constexpr basic_utf32_string& insert(size_type index, size_type count, utf32_char ch)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-32 character boundary");
		}

		base_type inserted{ base_.get_allocator() };
		const auto sv = details::utf32_char_view(ch);
		for (size_type i = 0; i != count; ++i)
		{
			inserted.append(sv);
		}

		base_.insert(index, inserted);
		return *this;
	}

	constexpr basic_utf32_string& insert_range(size_type index, views::utf32_view rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-32 character boundary");
		}

		return insert_code_points(index, rg.base());
	}

	constexpr basic_utf32_string& insert_range(size_type index, views::utf8_view rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-32 character boundary");
		}

		base_type inserted{ base_.get_allocator() };
		inserted.reserve(rg.base().size());
		for (utf8_char ch : rg)
		{
			std::array<char32_t, 2> encoded{};
			const auto encoded_count = ch.encode_utf32<char32_t>(encoded.begin());
			inserted.append(encoded.data(), encoded.data() + encoded_count);
		}

		base_.insert(index, inserted);
		return *this;
	}

	template <details::container_compatible_range<utf32_char> R>
	constexpr basic_utf32_string& insert_range(size_type index, R&& rg)
	{
		if (index > size()) [[unlikely]]
		{
			throw std::out_of_range("insert index out of range");
		}

		if (!this->is_char_boundary(index)) [[unlikely]]
		{
			throw std::out_of_range("insert index must be at a UTF-32 character boundary");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf32_char_range
		{
			utf32_char ch;

			constexpr auto begin() const noexcept
			{
				return details::utf32_char_view(ch).begin();
			}

			constexpr auto end() const noexcept
			{
				return details::utf32_char_view(ch).end();
			}
		};

		auto inserted = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf32_char_range{ static_cast<utf32_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.insert_range(base_.begin() + static_cast<difference_type>(index), inserted);
#else
		const utf32_string inserted(std::from_range, std::forward<R>(rg));
		base_.insert(index, inserted.base());
#endif
		return *this;
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	constexpr basic_utf32_string& insert(size_type index, It first, Sent last)
	{
		return insert_range(index, std::ranges::subrange(std::move(first), std::move(last)));
	}

	constexpr basic_utf32_string& insert(size_type index, std::initializer_list<utf32_char> ilist)
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
		const auto code_points_to_remove = removed.code_unit_count();
		const auto where_idx = base_.size() - code_points_to_remove;
		base_.erase(where_idx, code_points_to_remove);
		return removed;
	}

	constexpr basic_utf32_string& erase(size_type index, size_type count = npos)
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
			throw std::out_of_range("erase range must be a valid UTF-32 substring");
		}

		base_.erase(index, erase_count);
		return *this;
	}

	constexpr basic_utf32_string& reverse() noexcept
	{
		return reverse_code_points_unchecked(0, size());
	}

	constexpr basic_utf32_string& reverse_graphemes() noexcept
	{
		return reverse_grapheme_code_points_unchecked(0, size());
	}

	constexpr basic_utf32_string& reverse(size_type pos, size_type count = npos)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("reverse index out of range");
		}

		const auto remaining = size() - pos;
		const auto reverse_count = count == npos ? remaining : count;
		if (reverse_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("reverse count out of range");
		}

		const auto end = pos + reverse_count;
		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("reverse range must be a valid UTF-32 substring");
		}

		return reverse_code_points_unchecked(pos, reverse_count);
	}

	constexpr basic_utf32_string& reverse_graphemes(size_type pos, size_type count = npos)
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("reverse_graphemes index out of range");
		}

		const auto remaining = size() - pos;
		const auto reverse_count = count == npos ? remaining : count;
		if (reverse_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("reverse_graphemes count out of range");
		}

		const auto end = pos + reverse_count;
		if (!this->is_grapheme_boundary(pos) || !this->is_grapheme_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("reverse_graphemes range must be a valid UTF-32 grapheme substring");
		}

		return reverse_grapheme_code_points_unchecked(pos, reverse_count);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_lowercase() const&
	{
		return static_cast<const crtp&>(*this).template to_ascii_lowercase<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_lowercase(size_type pos, size_type count) const&
	{
		return static_cast<const crtp&>(*this).template to_ascii_lowercase<Allocator>(pos, count, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_lowercase() && noexcept
	{
		details::ascii_lowercase_inplace(base_.data(), base_.size());
		return std::move(*this);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_lowercase(size_type pos, size_type count) &&
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("case transform index out of range");
		}

		const auto remaining = size() - pos;
		const auto transform_count = count == npos ? remaining : count;
		if (transform_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("case transform count out of range");
		}

		const auto end = pos + transform_count;
		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("case transform range must be a valid UTF-32 substring");
		}

		details::ascii_lowercase_inplace(base_.data() + pos, transform_count);
		return std::move(*this);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_ascii_lowercase(const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_ascii_lowercase<OtherAllocator>(alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_ascii_lowercase(
		size_type pos,
		size_type count,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_ascii_lowercase<OtherAllocator>(pos, count, alloc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_uppercase() const&
	{
		return static_cast<const crtp&>(*this).template to_ascii_uppercase<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_uppercase(size_type pos, size_type count) const&
	{
		return static_cast<const crtp&>(*this).template to_ascii_uppercase<Allocator>(pos, count, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_uppercase() && noexcept
	{
		details::ascii_uppercase_inplace(base_.data(), base_.size());
		return std::move(*this);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_ascii_uppercase(size_type pos, size_type count) &&
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("case transform index out of range");
		}

		const auto remaining = size() - pos;
		const auto transform_count = count == npos ? remaining : count;
		if (transform_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("case transform count out of range");
		}

		const auto end = pos + transform_count;
		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("case transform range must be a valid UTF-32 substring");
		}

		details::ascii_uppercase_inplace(base_.data() + pos, transform_count);
		return std::move(*this);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_ascii_uppercase(const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_ascii_uppercase<OtherAllocator>(alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_ascii_uppercase(
		size_type pos,
		size_type count,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_ascii_uppercase<OtherAllocator>(pos, count, alloc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_lowercase() const&
	{
		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_lowercase(size_type pos, size_type count) const&
	{
		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(pos, count, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_lowercase() &&
	{
		if (details::is_ascii_only(std::u32string_view{ base_ }))
		{
			details::ascii_lowercase_inplace(base_.data(), base_.size());
			return std::move(*this);
		}

		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_lowercase(size_type pos, size_type count) &&
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("case transform index out of range");
		}

		const auto remaining = size() - pos;
		const auto transform_count = count == npos ? remaining : count;
		if (transform_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("case transform count out of range");
		}

		const auto end = pos + transform_count;
		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("case transform range must be a valid UTF-32 substring");
		}

		if (details::is_ascii_only(std::u32string_view{ base_.data() + pos, transform_count }))
		{
			details::ascii_lowercase_inplace(base_.data() + pos, transform_count);
			return std::move(*this);
		}

		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(pos, count, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_lowercase(const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_lowercase<OtherAllocator>(alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_lowercase(
		size_type pos,
		size_type count,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_lowercase<OtherAllocator>(pos, count, alloc);
	}

#if UTF8_RANGES_HAS_ICU
	[[nodiscard]]
	basic_utf32_string to_lowercase(locale_id locale) const&
	{
		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_lowercase(size_type pos, size_type count, locale_id locale) const&
	{
		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(pos, count, locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_lowercase(locale_id locale) &&
	{
		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_lowercase(size_type pos, size_type count, locale_id locale) &&
	{
		return static_cast<const crtp&>(*this).template to_lowercase<Allocator>(pos, count, locale, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	basic_utf32_string<OtherAllocator> to_lowercase(locale_id locale, const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_lowercase<OtherAllocator>(locale, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	basic_utf32_string<OtherAllocator> to_lowercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_lowercase<OtherAllocator>(pos, count, locale, alloc);
	}

	[[nodiscard]]
	basic_utf32_string to_uppercase(locale_id locale) const&
	{
		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_uppercase(size_type pos, size_type count, locale_id locale) const&
	{
		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(pos, count, locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_uppercase(locale_id locale) &&
	{
		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_uppercase(size_type pos, size_type count, locale_id locale) &&
	{
		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(pos, count, locale, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	basic_utf32_string<OtherAllocator> to_uppercase(locale_id locale, const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_uppercase<OtherAllocator>(locale, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	basic_utf32_string<OtherAllocator> to_uppercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_uppercase<OtherAllocator>(pos, count, locale, alloc);
	}

	[[nodiscard]]
	basic_utf32_string to_titlecase(locale_id locale) const&
	{
		return static_cast<const crtp&>(*this).template to_titlecase<Allocator>(locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string to_titlecase(locale_id locale) &&
	{
		return static_cast<const crtp&>(*this).template to_titlecase<Allocator>(locale, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	basic_utf32_string<OtherAllocator> to_titlecase(locale_id locale, const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_titlecase<OtherAllocator>(locale, alloc);
	}

	[[nodiscard]]
	basic_utf32_string case_fold(locale_id locale) const&
	{
		return static_cast<const crtp&>(*this).template case_fold<Allocator>(locale, base_.get_allocator());
	}

	[[nodiscard]]
	basic_utf32_string case_fold(locale_id locale) &&
	{
		return static_cast<const crtp&>(*this).template case_fold<Allocator>(locale, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	basic_utf32_string<OtherAllocator> case_fold(locale_id locale, const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template case_fold<OtherAllocator>(locale, alloc);
	}
#endif

	[[nodiscard]]
	constexpr basic_utf32_string to_uppercase() const&
	{
		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_uppercase(size_type pos, size_type count) const&
	{
		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(pos, count, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_uppercase() &&
	{
		if (details::is_ascii_only(std::u32string_view{ base_ }))
		{
			details::ascii_uppercase_inplace(base_.data(), base_.size());
			return std::move(*this);
		}

		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_uppercase(size_type pos, size_type count) &&
	{
		if (pos > size()) [[unlikely]]
		{
			throw std::out_of_range("case transform index out of range");
		}

		const auto remaining = size() - pos;
		const auto transform_count = count == npos ? remaining : count;
		if (transform_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("case transform count out of range");
		}

		const auto end = pos + transform_count;
		if (!this->is_char_boundary(pos) || !this->is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("case transform range must be a valid UTF-32 substring");
		}

		if (details::is_ascii_only(std::u32string_view{ base_.data() + pos, transform_count }))
		{
			details::ascii_uppercase_inplace(base_.data() + pos, transform_count);
			return std::move(*this);
		}

		return static_cast<const crtp&>(*this).template to_uppercase<Allocator>(pos, count, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_uppercase(const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_uppercase<OtherAllocator>(alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_uppercase(
		size_type pos,
		size_type count,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template to_uppercase<OtherAllocator>(pos, count, alloc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string normalize(normalization_form form) const&
	{
		return static_cast<const crtp&>(*this).template normalize<Allocator>(form, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string normalize(normalization_form form) &&
	{
		if (details::is_ascii_only(std::u32string_view{ base_ }))
		{
			return std::move(*this);
		}

		return static_cast<const crtp&>(*this).template normalize<Allocator>(form, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> normalize(
		normalization_form form,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template normalize<OtherAllocator>(form, alloc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfc() const&
	{
		return normalize(normalization_form::nfc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfc() &&
	{
		return std::move(*this).normalize(normalization_form::nfc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfd() const&
	{
		return normalize(normalization_form::nfd);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfd() &&
	{
		return std::move(*this).normalize(normalization_form::nfd);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfkc() const&
	{
		return normalize(normalization_form::nfkc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfkc() &&
	{
		return std::move(*this).normalize(normalization_form::nfkc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfkd() const&
	{
		return normalize(normalization_form::nfkd);
	}

	[[nodiscard]]
	constexpr basic_utf32_string to_nfkd() &&
	{
		return std::move(*this).normalize(normalization_form::nfkd);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_nfc(const OtherAllocator& alloc) const
	{
		return normalize(normalization_form::nfc, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_nfd(const OtherAllocator& alloc) const
	{
		return normalize(normalization_form::nfd, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_nfkc(const OtherAllocator& alloc) const
	{
		return normalize(normalization_form::nfkc, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> to_nfkd(const OtherAllocator& alloc) const
	{
		return normalize(normalization_form::nfkd, alloc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string case_fold() const&
	{
		return static_cast<const crtp&>(*this).template case_fold<Allocator>(base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string case_fold() &&
	{
		if (details::is_ascii_only(std::u32string_view{ base_ }))
		{
			details::ascii_lowercase_inplace(base_.data(), base_.size());
			return std::move(*this);
		}

		return static_cast<const crtp&>(*this).template case_fold<Allocator>(base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> case_fold(const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template case_fold<OtherAllocator>(alloc);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_char from, utf32_char to) const&
	{
		return replace_all(
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)),
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_char from, utf32_string_view to) const&
	{
		return replace_all(equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)), to);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_string_view from, utf32_char to) const&
	{
		return replace_all(from, equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_string_view from, utf32_string_view to) const&
	{
		return replace_code_points_copy(from.base(), to.base(), npos, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_char from, utf32_char to) &&
	{
		return std::move(*this).replace_all(
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)),
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_char from, utf32_string_view to) &&
	{
		return std::move(*this).replace_all(
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)),
			to);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_string_view from, utf32_char to) &&
	{
		return std::move(*this).replace_all(
			from,
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(utf32_string_view from, utf32_string_view to) &&
	{
		replace_code_points_in_place(from.base(), to.base(), npos);
		return std::move(*this);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(std::span<const utf32_char> from, utf32_char to) const&
	{
		return static_cast<const crtp&>(*this).template replace_all<Allocator>(from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(std::span<const utf32_char> from, utf32_string_view to) const&
	{
		return static_cast<const crtp&>(*this).template replace_all<Allocator>(from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(std::span<const utf32_char> from, utf32_char to) &&
	{
		if (from.empty())
		{
			return std::move(*this);
		}

		if (from.size() == 1)
		{
			return std::move(*this).replace_all(from.front(), to);
		}

		return static_cast<const crtp&>(*this).template replace_all<Allocator>(from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_all(std::span<const utf32_char> from, utf32_string_view to) &&
	{
		if (from.empty())
		{
			return std::move(*this);
		}

		if (from.size() == 1)
		{
			return std::move(*this).replace_all(from.front(), to);
		}

		return static_cast<const crtp&>(*this).template replace_all<Allocator>(from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_char from, utf32_char to) const&
	{
		return replace_n(
			count,
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)),
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_char from, utf32_string_view to) const&
	{
		return replace_n(count, equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)), to);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_string_view from, utf32_char to) const&
	{
		return replace_n(count, from, equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_string_view from, utf32_string_view to) const&
	{
		return replace_code_points_copy(from.base(), to.base(), count, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_char from, utf32_char to) &&
	{
		return std::move(*this).replace_n(
			count,
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)),
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_char from, utf32_string_view to) &&
	{
		return std::move(*this).replace_n(
			count,
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(from)),
			to);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_string_view from, utf32_char to) &&
	{
		return std::move(*this).replace_n(
			count,
			from,
			equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, utf32_string_view from, utf32_string_view to) &&
	{
		replace_code_points_in_place(from.base(), to.base(), count);
		return std::move(*this);
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, std::span<const utf32_char> from, utf32_char to) const&
	{
		return static_cast<const crtp&>(*this).template replace_n<Allocator>(count, from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, std::span<const utf32_char> from, utf32_string_view to) const&
	{
		return static_cast<const crtp&>(*this).template replace_n<Allocator>(count, from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, std::span<const utf32_char> from, utf32_char to) &&
	{
		if (count == 0 || from.empty())
		{
			return std::move(*this);
		}

		if (from.size() == 1)
		{
			return std::move(*this).replace_n(count, from.front(), to);
		}

		return static_cast<const crtp&>(*this).template replace_n<Allocator>(count, from, to, base_.get_allocator());
	}

	[[nodiscard]]
	constexpr basic_utf32_string replace_n(size_type count, std::span<const utf32_char> from, utf32_string_view to) &&
	{
		if (count == 0 || from.empty())
		{
			return std::move(*this);
		}

		if (from.size() == 1)
		{
			return std::move(*this).replace_n(count, from.front(), to);
		}

		return static_cast<const crtp&>(*this).template replace_n<Allocator>(count, from, to, base_.get_allocator());
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		utf32_char from,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		utf32_char from,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		utf32_string_view from,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		utf32_string_view from,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		std::span<const utf32_char> from,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		std::span<const utf32_char> from,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<OtherAllocator>(from, to, alloc);
	}

	template <details::utf32_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		Pred pred,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<std::remove_cvref_t<Pred>, OtherAllocator>(
			std::move(pred),
			to,
			alloc);
	}

	template <details::utf32_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_all(
		Pred pred,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_all<std::remove_cvref_t<Pred>, OtherAllocator>(
			std::move(pred),
			to,
			alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		utf32_char from,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		utf32_char from,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		utf32_string_view from,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		utf32_string_view from,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		std::span<const utf32_char> from,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		std::span<const utf32_char> from,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<OtherAllocator>(count, from, to, alloc);
	}

	template <details::utf32_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		Pred pred,
		utf32_char to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<std::remove_cvref_t<Pred>, OtherAllocator>(
			count,
			std::move(pred),
			to,
			alloc);
	}

	template <details::utf32_char_predicate Pred, typename OtherAllocator>
	[[nodiscard]]
	constexpr basic_utf32_string<OtherAllocator> replace_n(
		size_type count,
		Pred pred,
		utf32_string_view to,
		const OtherAllocator& alloc) const
	{
		return static_cast<const crtp&>(*this).template replace_n<std::remove_cvref_t<Pred>, OtherAllocator>(
			count,
			std::move(pred),
			to,
			alloc);
	}

	constexpr basic_utf32_string& replace_inplace(size_type pos, size_type count, utf32_string_view other)
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
			throw std::out_of_range("replace range must be a valid UTF-32 substring");
		}

		return replace_code_points(pos, replace_count, other.base());
	}

	constexpr basic_utf32_string& replace_inplace(size_type pos, size_type count, utf32_char other)
	{
		return replace_inplace(pos, count, equivalent_utf32_string_view::from_code_points_unchecked(details::utf32_char_view(other)));
	}

	constexpr basic_utf32_string& replace_inplace(size_type pos, utf32_string_view other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-32 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_code_points(pos, replace_count, other.base());
	}

	constexpr basic_utf32_string& replace_inplace(size_type pos, utf32_char other)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-32 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		base_.replace(pos, replace_count, details::utf32_char_view(other));
		return *this;
	}

	constexpr basic_utf32_string& replace_with_range_inplace(size_type pos, size_type count, views::utf32_view rg)
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
			throw std::out_of_range("replace range must be a valid UTF-32 substring");
		}

		return replace_code_points(pos, replace_count, rg.base());
	}

	constexpr basic_utf32_string& replace_with_range_inplace(size_type pos, size_type count, views::utf8_view rg)
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
			throw std::out_of_range("replace range must be a valid UTF-32 substring");
		}

		base_type replacement{ base_.get_allocator() };
		replacement.reserve(rg.base().size());
		for (utf8_char ch : rg)
		{
			std::array<char32_t, 2> encoded{};
			const auto encoded_count = ch.encode_utf32<char32_t>(encoded.begin());
			replacement.append(encoded.data(), encoded.data() + encoded_count);
		}

		base_.replace(pos, replace_count, replacement);
		return *this;
	}

	template <details::container_compatible_range<utf32_char> R>
	constexpr basic_utf32_string& replace_with_range_inplace(size_type pos, size_type count, R&& rg)
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
			throw std::out_of_range("replace range must be a valid UTF-32 substring");
		}

#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf32_char_range
		{
			utf32_char ch;

			constexpr auto begin() const noexcept
			{
				return details::utf32_char_view(ch).begin();
			}

			constexpr auto end() const noexcept
			{
				return details::utf32_char_view(ch).end();
			}
		};

		auto replacement = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf32_char_range{ static_cast<utf32_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(end),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf32_char ch : std::forward<R>(rg))
		{
			replacement.append(details::utf32_char_view(ch));
		}

		base_.replace(pos, replace_count, replacement);
#endif
		return *this;
	}

	constexpr basic_utf32_string& replace_with_range_inplace(size_type pos, views::utf32_view rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-32 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_with_range_inplace(pos, replace_count, rg);
	}

	constexpr basic_utf32_string& replace_with_range_inplace(size_type pos, views::utf8_view rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-32 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
		return replace_with_range_inplace(pos, replace_count, rg);
	}

	template <details::container_compatible_range<utf32_char> R>
	constexpr basic_utf32_string& replace_with_range_inplace(size_type pos, R&& rg)
	{
		if (pos >= size()) [[unlikely]]
		{
			throw std::out_of_range("replace index out of range");
		}

		if (!this->is_char_boundary(pos)) [[unlikely]]
		{
			throw std::out_of_range("replace index must be at a UTF-32 character boundary");
		}

		const auto replace_count = this->char_at_unchecked(pos).code_unit_count();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
		struct encoded_utf32_char_range
		{
			utf32_char ch;

			constexpr auto begin() const noexcept
			{
				return details::utf32_char_view(ch).begin();
			}

			constexpr auto end() const noexcept
			{
				return details::utf32_char_view(ch).end();
			}
		};

		auto replacement = std::forward<R>(rg)
			| std::views::transform([](auto&& ch)
				{
					return encoded_utf32_char_range{ static_cast<utf32_char>(std::forward<decltype(ch)>(ch)) };
				})
			| std::views::join;

		base_.replace_with_range(
			base_.begin() + static_cast<difference_type>(pos),
			base_.begin() + static_cast<difference_type>(pos + replace_count),
			replacement);
#else
		base_type replacement{ base_.get_allocator() };
		for (utf32_char ch : std::forward<R>(rg))
		{
			replacement.append(details::utf32_char_view(ch));
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
	constexpr const char32_t* data() const noexcept
	{
		return base_.data();
	}

	[[nodiscard]]
	constexpr const char32_t* c_str() const noexcept
	{
		return data();
	}

	constexpr operator utf32_string_view() const noexcept
	{
		return as_view();
	}

	[[nodiscard]]
	constexpr equivalent_utf32_string_view as_view() const noexcept
	{
		return equivalent_utf32_string_view::from_code_points_unchecked(equivalent_string_view{ base_ });
	}

	constexpr void push_back(utf32_char ch)
	{
		base_ += equivalent_string_view{ details::utf32_char_view(ch) };
	}

	constexpr void swap(basic_utf32_string& other)
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
			std::allocator_traits<Allocator>::is_always_equal::value)
	{
		base_.swap(other.base_);
	}

	friend constexpr bool operator==(const basic_utf32_string& lhs, const basic_utf32_string& rhs) noexcept
	{
		return lhs.base_ == rhs.base_;
	}

	friend constexpr bool operator==(const basic_utf32_string& lhs, utf32_string_view rhs) noexcept
	{
		return lhs.base_ == rhs.base();
	}

	friend constexpr bool operator==(utf32_string_view lhs, const basic_utf32_string& rhs) noexcept
	{
		return lhs.base() == rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf32_string& lhs, const basic_utf32_string& rhs) noexcept
	{
		return lhs.base_ <=> rhs.base_;
	}

	friend constexpr auto operator<=>(const basic_utf32_string& lhs, utf32_string_view rhs) noexcept
	{
		return lhs.base_ <=> rhs.base();
	}

	friend constexpr auto operator<=>(utf32_string_view lhs, const basic_utf32_string& rhs) noexcept
	{
		return lhs.base() <=> rhs.base_;
	}

	friend constexpr basic_utf32_string operator+(const basic_utf32_string& lhs, const basic_utf32_string& rhs)
	{
		return from_code_points_unchecked(lhs.base_ + rhs.base_);
	}

	friend constexpr basic_utf32_string operator+(basic_utf32_string&& lhs, const basic_utf32_string& rhs)
	{
		return from_code_points_unchecked(std::move(lhs.base_) + rhs.base_);
	}

	friend constexpr basic_utf32_string operator+(const basic_utf32_string& lhs, basic_utf32_string&& rhs)
	{
		return from_code_points_unchecked(lhs.base_ + std::move(rhs.base_));
	}

	friend constexpr basic_utf32_string operator+(basic_utf32_string&& lhs, basic_utf32_string&& rhs)
	{
		return from_code_points_unchecked(std::move(lhs.base_) + std::move(rhs.base_));
	}

	friend constexpr basic_utf32_string operator+(const basic_utf32_string& lhs, utf32_string_view rhs)
	{
		return from_code_points_unchecked(lhs.base_ + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf32_string operator+(basic_utf32_string&& lhs, utf32_string_view rhs)
	{
		return from_code_points_unchecked(std::move(lhs.base_) + base_type{ rhs.base(), lhs.get_allocator() });
	}

	friend constexpr basic_utf32_string operator+(utf32_string_view lhs, const basic_utf32_string& rhs)
	{
		return from_code_points_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf32_string operator+(utf32_string_view lhs, basic_utf32_string&& rhs)
	{
		return from_code_points_unchecked(base_type{ lhs.base(), rhs.get_allocator() } + std::move(rhs.base_));
	}

	friend constexpr basic_utf32_string operator+(const basic_utf32_string& lhs, utf32_char rhs)
	{
		return from_code_points_unchecked(lhs.base_ + base_type{ details::utf32_char_view(rhs), lhs.get_allocator() });
	}

	friend constexpr basic_utf32_string operator+(basic_utf32_string&& lhs, utf32_char rhs)
	{
		return from_code_points_unchecked(std::move(lhs.base_) + base_type{ details::utf32_char_view(rhs), lhs.get_allocator() });
	}

	friend constexpr basic_utf32_string operator+(utf32_char lhs, const basic_utf32_string& rhs)
	{
		return from_code_points_unchecked(base_type{ details::utf32_char_view(lhs), rhs.get_allocator() } + rhs.base_);
	}

	friend constexpr basic_utf32_string operator+(utf32_char lhs, basic_utf32_string&& rhs)
	{
		return from_code_points_unchecked(base_type{ details::utf32_char_view(lhs), rhs.get_allocator() } + std::move(rhs.base_));
	}

private:
	using base_class = details::utf32_string_crtp<basic_utf32_string<Allocator>, utf32_string_view>;

	constexpr explicit basic_utf32_string(base_type&& code_points) noexcept
		: base_(std::move(code_points))
	{
	}

	base_type base_;
};

template <typename Allocator>
inline std::ostream& operator<<(std::ostream& os, const basic_utf32_string<Allocator>& value)
{
	return os << value.as_view();
}

}

namespace std
{
	template<typename Allocator>
	struct formatter<unicode_ranges::basic_utf32_string<Allocator>, char> : formatter<unicode_ranges::utf32_string_view, char>
	{
		template<typename FormatContext>
		auto format(const unicode_ranges::basic_utf32_string<Allocator>& value, FormatContext& ctx) const
		{
			return formatter<unicode_ranges::utf32_string_view, char>::format(value.as_view(), ctx);
		}
	};

	template<typename Allocator, typename OtherAllocator>
	struct uses_allocator<unicode_ranges::basic_utf32_string<Allocator>, OtherAllocator> : true_type
	{
	};
}

namespace unicode_ranges
{

namespace literals
{
	template<details::literals::constexpr_utf32_string Str>
	constexpr auto operator ""_utf32_s()
	{
		const auto sv = std::u32string_view{ Str.data(), decltype(Str)::SIZE - 1 };
		const auto result = utf32_string_view::from_code_points(sv);
		if (!result)
		{
			throw std::invalid_argument("literal must contain only valid UTF-32");
		}
		return utf32_string(result.value());
	}
}

}

#include "utf8_string.hpp"
#include "transcoding.hpp"
#include "transcoding_utf32.hpp"

#endif // UTF8_RANGES_UTF32_STRING_HPP
