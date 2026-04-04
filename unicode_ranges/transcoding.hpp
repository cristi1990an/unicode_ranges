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
					const auto remaining = std::u16string_view{ code_units.data() + read_index, code_units.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf16_to_utf8(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					const auto scalar = details::decode_valid_utf16_char(code_units.data() + read_index, count);
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
					const auto remaining = std::u16string_view{ code_units.data() + read_index, code_units.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf16_to_utf8(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					const auto scalar = details::decode_valid_utf16_char(code_units.data() + read_index, count);
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
					const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf8_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(bytes.data() + read_index, count);
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
					const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf8_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(bytes.data() + read_index, count);
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
	inline constexpr std::size_t unicode_scalar_utf8_size(std::uint32_t scalar) noexcept
	{
		if (scalar <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			return encoding_constants::single_code_unit_count;
		}

		if (scalar <= encoding_constants::two_byte_scalar_max)
		{
			return encoding_constants::two_code_unit_count;
		}

		if (scalar <= encoding_constants::bmp_scalar_max)
		{
			return encoding_constants::three_code_unit_count;
		}

		return encoding_constants::max_utf8_code_units;
	}

	inline constexpr std::size_t unicode_scalar_utf16_size(std::uint32_t scalar) noexcept
	{
		return scalar <= encoding_constants::bmp_scalar_max
			? encoding_constants::single_code_unit_count
			: encoding_constants::utf16_surrogate_code_unit_count;
	}

	inline constexpr bool normalization_uses_compatibility(normalization_form form) noexcept
	{
		return form == normalization_form::nfkc || form == normalization_form::nfkd;
	}

	inline constexpr bool normalization_uses_composition(normalization_form form) noexcept
	{
		return form == normalization_form::nfc || form == normalization_form::nfkc;
	}

	inline constexpr bool is_hangul_syllable(std::uint32_t scalar) noexcept
	{
		return scalar >= 0xAC00u && scalar < 0xD7A4u;
	}

	inline constexpr bool is_hangul_l_jamo(std::uint32_t scalar) noexcept
	{
		return scalar >= 0x1100u && scalar < 0x1113u;
	}

	inline constexpr bool is_hangul_v_jamo(std::uint32_t scalar) noexcept
	{
		return scalar >= 0x1161u && scalar < 0x1176u;
	}

	inline constexpr bool is_hangul_t_jamo(std::uint32_t scalar) noexcept
	{
		return scalar > 0x11A7u && scalar < 0x11C3u;
	}

	inline constexpr void append_reordered_scalar(
		std::u32string& scalars,
		std::size_t& segment_begin,
		std::uint32_t scalar)
	{
		const auto combining_class = unicode::canonical_combining_class(scalar);
		scalars.push_back(static_cast<char32_t>(scalar));
		if (scalars.size() == 1)
		{
			segment_begin = 0;
			return;
		}

		if (combining_class == 0)
		{
			segment_begin = scalars.size() - 1;
			return;
		}

		std::size_t insert_index = scalars.size() - 1;
		while (insert_index > segment_begin)
		{
			const auto previous = static_cast<std::uint32_t>(scalars[insert_index - 1]);
			const auto previous_class = unicode::canonical_combining_class(previous);
			if (previous_class == 0 || previous_class <= combining_class)
			{
				break;
			}

			std::swap(scalars[insert_index], scalars[insert_index - 1]);
			--insert_index;
		}
	}

	template <bool Compatibility>
	constexpr void append_decomposed_scalar(
		std::u32string& scalars,
		std::size_t& segment_begin,
		std::uint32_t scalar)
	{
		if (is_hangul_syllable(scalar))
		{
			const auto s_index = scalar - 0xAC00u;
			const auto l = 0x1100u + (s_index / 588u);
			const auto v = 0x1161u + ((s_index % 588u) / 28u);
			const auto t = s_index % 28u;
			append_reordered_scalar(scalars, segment_begin, l);
			append_reordered_scalar(scalars, segment_begin, v);
			if (t != 0)
			{
				append_reordered_scalar(scalars, segment_begin, 0x11A7u + t);
			}
			return;
		}

		const auto mapping_index = unicode::decomposition_mapping_index(scalar);
		if (mapping_index != unicode::decomposition_mappings.size())
		{
			const auto& mapping = unicode::decomposition_mapping_at(mapping_index);
			if (!Compatibility && mapping.compatibility)
			{
				append_reordered_scalar(scalars, segment_begin, scalar);
				return;
			}

			for (std::size_t index = 0; index != mapping.count; ++index)
			{
				append_decomposed_scalar<Compatibility>(scalars, segment_begin, mapping.mapped[index]);
			}
			return;
		}

		append_reordered_scalar(scalars, segment_begin, scalar);
	}

	template <bool Compatibility>
	constexpr std::u32string decompose_scalars(std::u8string_view bytes)
	{
		std::u32string scalars;
		scalars.reserve(bytes.size());
		std::size_t segment_begin = 0;
		for (std::size_t index = 0; index < bytes.size();)
		{
			const auto decoded = decode_next_scalar(bytes, index);
			append_decomposed_scalar<Compatibility>(scalars, segment_begin, decoded.scalar);
			index = decoded.next_index;
		}
		return scalars;
	}

	template <bool Compatibility>
	constexpr std::u32string decompose_scalars(std::u16string_view code_units)
	{
		std::u32string scalars;
		scalars.reserve(code_units.size());
		std::size_t segment_begin = 0;
		for (std::size_t index = 0; index < code_units.size();)
		{
			const auto decoded = decode_next_scalar(code_units, index);
			append_decomposed_scalar<Compatibility>(scalars, segment_begin, decoded.scalar);
			index = decoded.next_index;
		}
		return scalars;
	}

	inline constexpr std::uint32_t try_compose_hangul(std::uint32_t first, std::uint32_t second) noexcept
	{
		if (is_hangul_l_jamo(first) && is_hangul_v_jamo(second))
		{
			return 0xAC00u + ((first - 0x1100u) * 21u + (second - 0x1161u)) * 28u;
		}

		if (is_hangul_syllable(first)
			&& ((first - 0xAC00u) % 28u == 0u)
			&& is_hangul_t_jamo(second))
		{
			return first + (second - 0x11A7u);
		}

		return 0u;
	}

	inline constexpr std::u32string compose_scalars(std::u32string_view decomposed)
	{
		std::u32string result;
		result.reserve(decomposed.size());

		std::size_t starter_index = static_cast<std::size_t>(-1);
		std::uint8_t last_combining_class = 0;

		for (char32_t ch : decomposed)
		{
			const auto scalar = static_cast<std::uint32_t>(ch);
			const auto combining_class = unicode::canonical_combining_class(scalar);
			if (result.empty())
			{
				result.push_back(ch);
				starter_index = combining_class == 0 ? 0 : static_cast<std::size_t>(-1);
				last_combining_class = combining_class;
				continue;
			}

			bool composed = false;
			if (starter_index != static_cast<std::size_t>(-1)
				&& (last_combining_class == 0 || last_combining_class < combining_class))
			{
				const auto starter = static_cast<std::uint32_t>(result[starter_index]);
				if (const auto hangul = try_compose_hangul(starter, scalar); hangul != 0)
				{
					result[starter_index] = static_cast<char32_t>(hangul);
					composed = true;
				}
				else
				{
					const auto mapping_index = unicode::composition_mapping_index(starter, scalar);
					if (mapping_index != unicode::composition_mappings.size())
					{
						result[starter_index] = static_cast<char32_t>(unicode::composition_mapping_at(mapping_index).composed);
						composed = true;
					}
				}
			}

			if (!composed)
			{
				result.push_back(ch);
				if (combining_class == 0)
				{
					starter_index = result.size() - 1;
				}
				last_combining_class = combining_class;
			}
		}

		return result;
	}

	template <typename InputView>
	constexpr std::u32string normalized_scalars(InputView input, normalization_form form)
	{
		const auto decomposed = normalization_uses_compatibility(form)
			? decompose_scalars<true>(input)
			: decompose_scalars<false>(input);

		if (!normalization_uses_composition(form))
		{
			return decomposed;
		}

		return compose_scalars(decomposed);
	}

	inline constexpr std::size_t scalar_sequence_utf8_size(std::u32string_view scalars) noexcept
	{
		std::size_t size = 0;
		for (char32_t ch : scalars)
		{
			size += unicode_scalar_utf8_size(static_cast<std::uint32_t>(ch));
		}
		return size;
	}

	inline constexpr std::size_t scalar_sequence_utf16_size(std::u32string_view scalars) noexcept
	{
		std::size_t size = 0;
		for (char32_t ch : scalars)
		{
			size += unicode_scalar_utf16_size(static_cast<std::uint32_t>(ch));
		}
		return size;
	}

	inline constexpr std::uint32_t lowercase_ascii_scalar(std::uint32_t scalar) noexcept
	{
		return (scalar >= static_cast<std::uint32_t>('A') && scalar <= static_cast<std::uint32_t>('Z'))
			? (scalar + (static_cast<std::uint32_t>('a') - static_cast<std::uint32_t>('A')))
			: scalar;
	}

	inline constexpr std::uint32_t uppercase_ascii_scalar(std::uint32_t scalar) noexcept
	{
		return (scalar >= static_cast<std::uint32_t>('a') && scalar <= static_cast<std::uint32_t>('z'))
			? (scalar - (static_cast<std::uint32_t>('a') - static_cast<std::uint32_t>('A')))
			: scalar;
	}

	template <bool Lowercase>
	inline constexpr std::uint32_t ascii_case_scalar(std::uint32_t scalar) noexcept
	{
		if constexpr (Lowercase)
		{
			return lowercase_ascii_scalar(scalar);
		}
		else
		{
			return uppercase_ascii_scalar(scalar);
		}
	}

	struct case_mapping_lookup_result
	{
		bool has_simple = false;
		std::uint32_t simple_mapped = 0;
		std::size_t special_index = static_cast<std::size_t>(-1);

		constexpr bool has_special() const noexcept
		{
			return special_index != static_cast<std::size_t>(-1);
		}
	};

	inline constexpr std::size_t case_special_mapping_utf8_size(
		const unicode::unicode_special_case_mapping& mapping) noexcept
	{
		std::size_t size = 0;
		for (std::size_t index = 0; index != mapping.count; ++index)
		{
			size += unicode_scalar_utf8_size(mapping.mapped[index]);
		}

		return size;
	}

	inline constexpr std::size_t case_special_mapping_utf16_size(
		const unicode::unicode_special_case_mapping& mapping) noexcept
	{
		std::size_t size = 0;
		for (std::size_t index = 0; index != mapping.count; ++index)
		{
			size += unicode_scalar_utf16_size(mapping.mapped[index]);
		}

		return size;
	}

	template <bool Lowercase>
	inline constexpr const unicode::unicode_special_case_mapping& case_special_mapping_from_index(
		std::size_t index) noexcept
	{
		if constexpr (Lowercase)
		{
			return unicode::lowercase_special_mapping_at(index);
		}
		else
		{
			return unicode::uppercase_special_mapping_at(index);
		}
	}

	inline constexpr const unicode::unicode_special_case_mapping& case_fold_special_mapping_from_index(
		std::size_t index) noexcept
	{
		return unicode::case_fold_special_mapping_at(index);
	}

	template <bool Lowercase>
	inline constexpr case_mapping_lookup_result lookup_case_mapping(std::uint32_t scalar) noexcept
	{
		if constexpr (Lowercase)
		{
			if (const auto* range = unicode::lowercase_simple_delta_range(scalar); range != nullptr)
			{
				return case_mapping_lookup_result{
					.has_simple = true,
					.simple_mapped = unicode::apply_case_mapping_delta(scalar, range->delta)
				};
			}

			if (const auto* mapping = unicode::lowercase_simple_mapping(scalar); mapping != nullptr)
			{
				return case_mapping_lookup_result{
					.has_simple = true,
					.simple_mapped = mapping->mapped
				};
			}

			const auto special_index = unicode::lowercase_special_mapping_index(scalar);
			return case_mapping_lookup_result{
				.special_index = special_index == unicode::lowercase_special_mappings.size()
					? static_cast<std::size_t>(-1)
					: special_index
			};
		}
		else
		{
			if (const auto* range = unicode::uppercase_simple_delta_range(scalar); range != nullptr)
			{
				return case_mapping_lookup_result{
					.has_simple = true,
					.simple_mapped = unicode::apply_case_mapping_delta(scalar, range->delta)
				};
			}

			if (const auto* mapping = unicode::uppercase_simple_mapping(scalar); mapping != nullptr)
			{
				return case_mapping_lookup_result{
					.has_simple = true,
					.simple_mapped = mapping->mapped
				};
			}

			const auto special_index = unicode::uppercase_special_mapping_index(scalar);
			return case_mapping_lookup_result{
				.special_index = special_index == unicode::uppercase_special_mappings.size()
					? static_cast<std::size_t>(-1)
					: special_index
			};
		}
	}

	inline constexpr case_mapping_lookup_result lookup_case_fold_mapping(std::uint32_t scalar) noexcept
	{
		if (const auto* range = unicode::case_fold_simple_delta_range(scalar); range != nullptr)
		{
			return case_mapping_lookup_result{
				.has_simple = true,
				.simple_mapped = unicode::apply_case_mapping_delta(scalar, range->delta)
			};
		}

		if (const auto* mapping = unicode::case_fold_simple_mapping(scalar); mapping != nullptr)
		{
			return case_mapping_lookup_result{
				.has_simple = true,
				.simple_mapped = mapping->mapped
			};
		}

		const auto special_index = unicode::case_fold_special_mapping_index(scalar);
		return case_mapping_lookup_result{
			.special_index = special_index == unicode::case_fold_special_mappings.size()
				? static_cast<std::size_t>(-1)
				: special_index
		};
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> copy_utf8_view(
		std::u8string_view bytes,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, bytes.data(), bytes.size());
				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> copy_utf16_view(
		std::u16string_view code_units,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, code_units.data(), code_units.size());
				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	struct case_map_measurement
	{
		bool changed = false;
		std::size_t output_size = 0;
	};

	template <bool Lowercase>
	constexpr case_map_measurement measure_case_map_utf8(std::u8string_view bytes) noexcept
	{
		case_map_measurement result{};
		for (std::size_t index = 0; index < bytes.size();)
		{
			const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				result.output_size += ascii_run;
				for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
				{
					const auto scalar = static_cast<std::uint8_t>(remaining[ascii_index]);
					result.changed = result.changed || (ascii_case_scalar<Lowercase>(scalar) != scalar);
				}

				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(bytes, index);
			const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
			if (mapping.has_simple)
			{
				result.changed = true;
				result.output_size += unicode_scalar_utf8_size(mapping.simple_mapped);
			}
			else if (mapping.has_special())
			{
				result.changed = true;
				const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
				for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
				{
					result.output_size += unicode_scalar_utf8_size(special.mapped[mapped_index]);
				}
			}
			else
			{
				result.output_size += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return result;
	}

	template <bool Lowercase>
	constexpr case_map_measurement measure_case_map_utf16(std::u16string_view code_units) noexcept
	{
		case_map_measurement result{};
		for (std::size_t index = 0; index < code_units.size();)
		{
			const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				result.output_size += ascii_run;
				for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
				{
					const auto scalar = static_cast<std::uint16_t>(remaining[ascii_index]);
					result.changed = result.changed || (ascii_case_scalar<Lowercase>(scalar) != scalar);
				}

				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(code_units, index);
			const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
			if (mapping.has_simple)
			{
				result.changed = true;
				result.output_size += unicode_scalar_utf16_size(mapping.simple_mapped);
			}
			else if (mapping.has_special())
			{
				result.changed = true;
				const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
				for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
				{
					result.output_size += unicode_scalar_utf16_size(special.mapped[mapped_index]);
				}
			}
			else
			{
				result.output_size += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return result;
	}

	struct utf8_case_transform_parts
	{
		std::u8string_view prefix;
		std::u8string_view middle;
		std::u8string_view suffix;
	};

	struct utf16_case_transform_parts
	{
		std::u16string_view prefix;
		std::u16string_view middle;
		std::u16string_view suffix;
	};

	template <typename Derived, typename View>
	constexpr utf8_case_transform_parts checked_utf8_case_transform_parts(
		const utf8_string_crtp<Derived, View>& self,
		typename utf8_string_crtp<Derived, View>::size_type pos,
		typename utf8_string_crtp<Derived, View>::size_type count)
	{
		const auto bytes = std::u8string_view{ static_cast<const Derived&>(self).base() };
		if (pos > bytes.size()) [[unlikely]]
		{
			throw std::out_of_range("case transform index out of range");
		}

		const auto remaining = bytes.size() - pos;
		const auto transform_count = count == utf8_string_crtp<Derived, View>::npos ? remaining : count;
		if (transform_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("case transform count out of range");
		}

		const auto end = pos + transform_count;
		if (!self.is_char_boundary(pos) || !self.is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("case transform range must be a valid UTF-8 substring");
		}

		return utf8_case_transform_parts{
			bytes.substr(0, pos),
			bytes.substr(pos, transform_count),
			bytes.substr(end)
		};
	}

	template <typename Derived, typename View>
	constexpr utf16_case_transform_parts checked_utf16_case_transform_parts(
		const utf16_string_crtp<Derived, View>& self,
		typename utf16_string_crtp<Derived, View>::size_type pos,
		typename utf16_string_crtp<Derived, View>::size_type count)
	{
		const auto code_units = std::u16string_view{ static_cast<const Derived&>(self).base() };
		if (pos > code_units.size()) [[unlikely]]
		{
			throw std::out_of_range("case transform index out of range");
		}

		const auto remaining = code_units.size() - pos;
		const auto transform_count = count == utf16_string_crtp<Derived, View>::npos ? remaining : count;
		if (transform_count > remaining) [[unlikely]]
		{
			throw std::out_of_range("case transform count out of range");
		}

		const auto end = pos + transform_count;
		if (!self.is_char_boundary(pos) || !self.is_char_boundary(end)) [[unlikely]]
		{
			throw std::out_of_range("case transform range must be a valid UTF-16 substring");
		}

		return utf16_case_transform_parts{
			code_units.substr(0, pos),
			code_units.substr(pos, transform_count),
			code_units.substr(end)
		};
	}

#if UTF8_RANGES_HAS_ICU
	struct ucasemap_closer
	{
		void operator()(UCaseMap* map) const noexcept
		{
			if (map != nullptr)
			{
				ucasemap_close(map);
			}
		}
	};

	inline std::string checked_icu_locale_name(locale_id locale)
	{
		if (locale.name.find('\0') != std::string_view::npos)
		{
			throw std::invalid_argument("locale_id must not contain embedded NUL");
		}

		return std::string{ locale.name };
	}

	inline int32_t checked_icu_length(std::size_t size, const char* what)
	{
		if (size > static_cast<std::size_t>((std::numeric_limits<int32_t>::max)()))
		{
			throw std::length_error(std::string{ what } + " is too large for ICU");
		}

		return static_cast<int32_t>(size);
	}

	[[noreturn]] inline void throw_icu_error(const char* operation, UErrorCode error)
	{
		throw std::runtime_error(std::string{ operation } + " failed: " + u_errorName(error));
	}

	inline std::unique_ptr<UCaseMap, ucasemap_closer> make_icu_case_map(locale_id locale)
	{
		const auto locale_name = checked_icu_locale_name(locale);
		UErrorCode error = U_ZERO_ERROR;
		auto map = std::unique_ptr<UCaseMap, ucasemap_closer>{ ucasemap_open(locale_name.c_str(), 0, &error) };
		if (U_FAILURE(error) || map == nullptr)
		{
			throw_icu_error("ucasemap_open", error);
		}

		return map;
	}

	template <typename Allocator>
	basic_utf8_string<Allocator> icu_lowercase_utf8_copy(
		std::u8string_view bytes,
		locale_id locale,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;

		if (bytes.empty())
		{
			return basic_utf8_string<Allocator>::from_bytes_unchecked(base_type{ alloc });
		}

		auto case_map = make_icu_case_map(locale);
		const auto input_size = checked_icu_length(bytes.size(), "UTF-8 input");
		UErrorCode error = U_ZERO_ERROR;
		const auto needed = ucasemap_utf8ToLower(
			case_map.get(),
			nullptr,
			0,
			reinterpret_cast<const char*>(bytes.data()),
			input_size,
			&error);

		if (error != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(error))
		{
			throw_icu_error("ucasemap_utf8ToLower", error);
		}

		base_type result{ alloc };
		if (needed != 0)
		{
			result.resize(static_cast<std::size_t>(needed));
			error = U_ZERO_ERROR;
			const auto written = ucasemap_utf8ToLower(
				case_map.get(),
				reinterpret_cast<char*>(result.data()),
				needed,
				reinterpret_cast<const char*>(bytes.data()),
				input_size,
				&error);
			if (U_FAILURE(error))
			{
				throw_icu_error("ucasemap_utf8ToLower", error);
			}

			result.resize(static_cast<std::size_t>(written));
		}

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Allocator>
	basic_utf8_string<Allocator> icu_uppercase_utf8_copy(
		std::u8string_view bytes,
		locale_id locale,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;

		if (bytes.empty())
		{
			return basic_utf8_string<Allocator>::from_bytes_unchecked(base_type{ alloc });
		}

		auto case_map = make_icu_case_map(locale);
		const auto input_size = checked_icu_length(bytes.size(), "UTF-8 input");
		UErrorCode error = U_ZERO_ERROR;
		const auto needed = ucasemap_utf8ToUpper(
			case_map.get(),
			nullptr,
			0,
			reinterpret_cast<const char*>(bytes.data()),
			input_size,
			&error);

		if (error != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(error))
		{
			throw_icu_error("ucasemap_utf8ToUpper", error);
		}

		base_type result{ alloc };
		if (needed != 0)
		{
			result.resize(static_cast<std::size_t>(needed));
			error = U_ZERO_ERROR;
			const auto written = ucasemap_utf8ToUpper(
				case_map.get(),
				reinterpret_cast<char*>(result.data()),
				needed,
				reinterpret_cast<const char*>(bytes.data()),
				input_size,
				&error);
			if (U_FAILURE(error))
			{
				throw_icu_error("ucasemap_utf8ToUpper", error);
			}

			result.resize(static_cast<std::size_t>(written));
		}

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Allocator>
	basic_utf16_string<Allocator> icu_lowercase_utf16_copy(
		std::u16string_view code_units,
		locale_id locale,
		const Allocator& alloc)
	{
		static_assert(sizeof(UChar) == sizeof(char16_t));

		using base_type = typename basic_utf16_string<Allocator>::base_type;

		if (code_units.empty())
		{
			return basic_utf16_string<Allocator>::from_code_units_unchecked(base_type{ alloc });
		}

		const auto locale_name = checked_icu_locale_name(locale);
		const auto input_size = checked_icu_length(code_units.size(), "UTF-16 input");
		const auto* source = reinterpret_cast<const UChar*>(code_units.data());
		UErrorCode error = U_ZERO_ERROR;
		const auto needed = u_strToLower(
			nullptr,
			0,
			source,
			input_size,
			locale_name.c_str(),
			&error);

		if (error != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(error))
		{
			throw_icu_error("u_strToLower", error);
		}

		base_type result{ alloc };
		if (needed != 0)
		{
			result.resize(static_cast<std::size_t>(needed));
			error = U_ZERO_ERROR;
			const auto written = u_strToLower(
				reinterpret_cast<UChar*>(result.data()),
				needed,
				source,
				input_size,
				locale_name.c_str(),
				&error);
			if (U_FAILURE(error))
			{
				throw_icu_error("u_strToLower", error);
			}

			result.resize(static_cast<std::size_t>(written));
		}

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Allocator>
	basic_utf16_string<Allocator> icu_uppercase_utf16_copy(
		std::u16string_view code_units,
		locale_id locale,
		const Allocator& alloc)
	{
		static_assert(sizeof(UChar) == sizeof(char16_t));

		using base_type = typename basic_utf16_string<Allocator>::base_type;

		if (code_units.empty())
		{
			return basic_utf16_string<Allocator>::from_code_units_unchecked(base_type{ alloc });
		}

		const auto locale_name = checked_icu_locale_name(locale);
		const auto input_size = checked_icu_length(code_units.size(), "UTF-16 input");
		const auto* source = reinterpret_cast<const UChar*>(code_units.data());
		UErrorCode error = U_ZERO_ERROR;
		const auto needed = u_strToUpper(
			nullptr,
			0,
			source,
			input_size,
			locale_name.c_str(),
			&error);

		if (error != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(error))
		{
			throw_icu_error("u_strToUpper", error);
		}

		base_type result{ alloc };
		if (needed != 0)
		{
			result.resize(static_cast<std::size_t>(needed));
			error = U_ZERO_ERROR;
			const auto written = u_strToUpper(
				reinterpret_cast<UChar*>(result.data()),
				needed,
				source,
				input_size,
				locale_name.c_str(),
				&error);
			if (U_FAILURE(error))
			{
				throw_icu_error("u_strToUpper", error);
			}

			result.resize(static_cast<std::size_t>(written));
		}

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}
#endif

	template <bool Lowercase>
	constexpr std::size_t write_case_map_utf8_into(std::u8string_view bytes, char8_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		for (std::size_t index = 0; index < bytes.size();)
		{
			const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				if constexpr (Lowercase)
				{
					ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
				}
				else
				{
					ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
				}

				write_index += ascii_run;
				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(bytes, index);
			const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
			if (mapping.has_simple)
			{
				write_index += encode_unicode_scalar_utf8_unchecked(mapping.simple_mapped, buffer + write_index);
			}
			else if (mapping.has_special())
			{
				const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
				for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
				{
					write_index += encode_unicode_scalar_utf8_unchecked(
						special.mapped[mapped_index],
						buffer + write_index);
				}
			}
			else
			{
				std::char_traits<char8_t>::copy(
					buffer + write_index,
					bytes.data() + index,
					decoded.next_index - index);
				write_index += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return write_index;
	}

	template <bool Lowercase>
	constexpr std::size_t write_case_map_utf16_into(std::u16string_view code_units, char16_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		for (std::size_t index = 0; index < code_units.size();)
		{
			const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				if constexpr (Lowercase)
				{
					ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
				}
				else
				{
					ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
				}

				write_index += ascii_run;
				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(code_units, index);
			const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
			if (mapping.has_simple)
			{
				write_index += encode_unicode_scalar_utf16_unchecked(mapping.simple_mapped, buffer + write_index);
			}
			else if (mapping.has_special())
			{
				const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
				for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
				{
					write_index += encode_unicode_scalar_utf16_unchecked(
						special.mapped[mapped_index],
						buffer + write_index);
				}
			}
			else
			{
				std::char_traits<char16_t>::copy(
					buffer + write_index,
					code_units.data() + index,
					decoded.next_index - index);
				write_index += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return write_index;
	}

	template <bool Lowercase, typename BaseType>
	constexpr void write_case_map_utf8(
		std::u8string_view bytes,
		BaseType& result,
		std::size_t output_size)
	{
		result.resize_and_overwrite(output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				return write_case_map_utf8_into<Lowercase>(bytes, buffer);
			});
	}

	template <bool Lowercase, typename BaseType>
	constexpr void write_case_map_utf16(
		std::u16string_view code_units,
		BaseType& result,
		std::size_t output_size)
	{
		result.resize_and_overwrite(output_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				return write_case_map_utf16_into<Lowercase>(code_units, buffer);
			});
	}

	template <bool Lowercase, typename BaseType>
	constexpr bool try_case_map_utf8_same_size(
		std::u8string_view bytes,
		BaseType& result)
	{
		bool same_size = true;
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t index = 0; index < bytes.size();)
				{
					const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (Lowercase)
						{
							ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}
						else
						{
							ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}

						write_index += ascii_run;
						index += ascii_run;
						continue;
					}

					const auto decoded = decode_next_scalar(bytes, index);
					const auto input_size = decoded.next_index - index;
					const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
					if (mapping.has_special())
					{
						const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
						if (case_special_mapping_utf8_size(special) != input_size)
						{
							same_size = false;
							return write_index;
						}

						for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
						{
							write_index += encode_unicode_scalar_utf8_unchecked(
								special.mapped[mapped_index],
								buffer + write_index);
						}
					}
					else if (mapping.has_simple)
					{
						if (unicode_scalar_utf8_size(mapping.simple_mapped) != input_size)
						{
							same_size = false;
							return write_index;
						}

						write_index += encode_unicode_scalar_utf8_unchecked(mapping.simple_mapped, buffer + write_index);
					}
					else
					{
						std::char_traits<char8_t>::copy(buffer + write_index, bytes.data() + index, input_size);
						write_index += input_size;
					}

					index = decoded.next_index;
				}

				return write_index;
			});

		return same_size;
	}

	template <bool Lowercase, typename BaseType>
	constexpr bool try_case_map_utf16_same_size(
		std::u16string_view code_units,
		BaseType& result)
	{
		bool same_size = true;
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t index = 0; index < code_units.size();)
				{
					const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (Lowercase)
						{
							ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}
						else
						{
							ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}

						write_index += ascii_run;
						index += ascii_run;
						continue;
					}

					const auto decoded = decode_next_scalar(code_units, index);
					const auto input_size = decoded.next_index - index;
					const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
					if (mapping.has_special())
					{
						const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
						if (case_special_mapping_utf16_size(special) != input_size)
						{
							same_size = false;
							return write_index;
						}

						for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
						{
							write_index += encode_unicode_scalar_utf16_unchecked(
								special.mapped[mapped_index],
								buffer + write_index);
						}
					}
					else if (mapping.has_simple)
					{
						if (unicode_scalar_utf16_size(mapping.simple_mapped) != input_size)
						{
							same_size = false;
							return write_index;
						}

						write_index += encode_unicode_scalar_utf16_unchecked(mapping.simple_mapped, buffer + write_index);
					}
					else
					{
						std::char_traits<char16_t>::copy(buffer + write_index, code_units.data() + index, input_size);
						write_index += input_size;
					}

					index = decoded.next_index;
				}

				return write_index;
			});

		return same_size;
	}

	template <bool Lowercase, typename Allocator>
	constexpr basic_utf8_string<Allocator> case_map_utf8_copy(
		std::u8string_view bytes,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;

		if (is_ascii_only(bytes))
		{
			bool changed = false;
			base_type result{ alloc };
			result.resize_and_overwrite(bytes.size(),
				[&](char8_t* buffer, std::size_t) noexcept
				{
					if constexpr (Lowercase)
					{
						changed = ascii_lowercase_copy(buffer, bytes);
					}
					else
					{
						changed = ascii_uppercase_copy(buffer, bytes);
					}

					return bytes.size();
				});

			if (!changed)
			{
				return copy_utf8_view(bytes, alloc);
			}

			return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
		}

		base_type same_size_result{ alloc };
		if (try_case_map_utf8_same_size<Lowercase>(bytes, same_size_result))
		{
			return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(same_size_result));
		}

		const auto measurement = measure_case_map_utf8<Lowercase>(bytes);
		if (!measurement.changed)
		{
			return copy_utf8_view(bytes, alloc);
		}

		base_type result{ alloc };
		write_case_map_utf8<Lowercase>(bytes, result, measurement.output_size);

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <bool Lowercase, typename Allocator>
	constexpr basic_utf16_string<Allocator> case_map_utf16_copy(
		std::u16string_view code_units,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;

		if (is_ascii_only(code_units))
		{
			bool changed = false;
			base_type result{ alloc };
			result.resize_and_overwrite(code_units.size(),
				[&](char16_t* buffer, std::size_t) noexcept
				{
					if constexpr (Lowercase)
					{
						changed = ascii_lowercase_copy(buffer, code_units);
					}
					else
					{
						changed = ascii_uppercase_copy(buffer, code_units);
					}

					return code_units.size();
				});

			if (!changed)
			{
				return copy_utf16_view(code_units, alloc);
			}

			return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
		}

		base_type same_size_result{ alloc };
		if (try_case_map_utf16_same_size<Lowercase>(code_units, same_size_result))
		{
			return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(same_size_result));
		}

		const auto measurement = measure_case_map_utf16<Lowercase>(code_units);
		if (!measurement.changed)
		{
			return copy_utf16_view(code_units, alloc);
		}

		base_type result{ alloc };
		write_case_map_utf16<Lowercase>(code_units, result, measurement.output_size);

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	constexpr case_map_measurement measure_case_fold_utf8(std::u8string_view bytes) noexcept
	{
		case_map_measurement result{};
		for (std::size_t index = 0; index < bytes.size();)
		{
			const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				result.output_size += ascii_run;
				for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
				{
					const auto scalar = static_cast<std::uint8_t>(remaining[ascii_index]);
					result.changed = result.changed || (lowercase_ascii_scalar(scalar) != scalar);
				}

				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(bytes, index);
			const auto mapping = lookup_case_fold_mapping(decoded.scalar);
			if (mapping.has_simple)
			{
				result.changed = true;
				result.output_size += unicode_scalar_utf8_size(mapping.simple_mapped);
			}
			else if (mapping.has_special())
			{
				result.changed = true;
				result.output_size += case_special_mapping_utf8_size(case_fold_special_mapping_from_index(mapping.special_index));
			}
			else
			{
				result.output_size += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return result;
	}

	constexpr case_map_measurement measure_case_fold_utf16(std::u16string_view code_units) noexcept
	{
		case_map_measurement result{};
		for (std::size_t index = 0; index < code_units.size();)
		{
			const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				result.output_size += ascii_run;
				for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
				{
					const auto scalar = static_cast<std::uint16_t>(remaining[ascii_index]);
					result.changed = result.changed || (lowercase_ascii_scalar(scalar) != scalar);
				}

				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(code_units, index);
			const auto mapping = lookup_case_fold_mapping(decoded.scalar);
			if (mapping.has_simple)
			{
				result.changed = true;
				result.output_size += unicode_scalar_utf16_size(mapping.simple_mapped);
			}
			else if (mapping.has_special())
			{
				result.changed = true;
				result.output_size += case_special_mapping_utf16_size(case_fold_special_mapping_from_index(mapping.special_index));
			}
			else
			{
				result.output_size += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return result;
	}

	constexpr std::size_t write_case_fold_utf8_into(std::u8string_view bytes, char8_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		for (std::size_t index = 0; index < bytes.size();)
		{
			const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
				write_index += ascii_run;
				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(bytes, index);
			const auto mapping = lookup_case_fold_mapping(decoded.scalar);
			if (mapping.has_simple)
			{
				write_index += encode_unicode_scalar_utf8_unchecked(mapping.simple_mapped, buffer + write_index);
			}
			else if (mapping.has_special())
			{
				const auto& special = case_fold_special_mapping_from_index(mapping.special_index);
				for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
				{
					write_index += encode_unicode_scalar_utf8_unchecked(
						special.mapped[mapped_index],
						buffer + write_index);
				}
			}
			else
			{
				std::char_traits<char8_t>::copy(buffer + write_index, bytes.data() + index, decoded.next_index - index);
				write_index += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return write_index;
	}

	constexpr std::size_t write_case_fold_utf16_into(std::u16string_view code_units, char16_t* buffer) noexcept
	{
		std::size_t write_index = 0;
		for (std::size_t index = 0; index < code_units.size();)
		{
			const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
				write_index += ascii_run;
				index += ascii_run;
				continue;
			}

			const auto decoded = decode_next_scalar(code_units, index);
			const auto mapping = lookup_case_fold_mapping(decoded.scalar);
			if (mapping.has_simple)
			{
				write_index += encode_unicode_scalar_utf16_unchecked(mapping.simple_mapped, buffer + write_index);
			}
			else if (mapping.has_special())
			{
				const auto& special = case_fold_special_mapping_from_index(mapping.special_index);
				for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
				{
					write_index += encode_unicode_scalar_utf16_unchecked(
						special.mapped[mapped_index],
						buffer + write_index);
				}
			}
			else
			{
				std::char_traits<char16_t>::copy(buffer + write_index, code_units.data() + index, decoded.next_index - index);
				write_index += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		return write_index;
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> case_fold_utf8_copy(
		std::u8string_view bytes,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;

		const auto measurement = measure_case_fold_utf8(bytes);
		if (!measurement.changed)
		{
			return copy_utf8_view(bytes, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(measurement.output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				return write_case_fold_utf8_into(bytes, buffer);
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> case_fold_utf16_copy(
		std::u16string_view code_units,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;

		const auto measurement = measure_case_fold_utf16(code_units);
		if (!measurement.changed)
		{
			return copy_utf16_view(code_units, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(measurement.output_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				return write_case_fold_utf16_into(code_units, buffer);
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> normalize_utf8_copy(
		std::u8string_view bytes,
		normalization_form form,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;

		if (bytes.empty())
		{
			return basic_utf8_string<Allocator>::from_bytes_unchecked(base_type{ alloc });
		}

		if (is_ascii_only(bytes))
		{
			return copy_utf8_view(bytes, alloc);
		}

		const auto normalized = normalized_scalars(bytes, form);
		base_type result{ alloc };
		result.resize_and_overwrite(scalar_sequence_utf8_size(normalized),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (char32_t ch : normalized)
				{
					write_index += encode_unicode_scalar_utf8_unchecked(static_cast<std::uint32_t>(ch), buffer + write_index);
				}
				return write_index;
			});

		if (result.size() == bytes.size()
			&& std::char_traits<char8_t>::compare(result.data(), bytes.data(), bytes.size()) == 0)
		{
			return copy_utf8_view(bytes, alloc);
		}

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> normalize_utf16_copy(
		std::u16string_view code_units,
		normalization_form form,
		const Allocator& alloc)
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;

		if (code_units.empty())
		{
			return basic_utf16_string<Allocator>::from_code_units_unchecked(base_type{ alloc });
		}

		if (is_ascii_only(code_units))
		{
			return copy_utf16_view(code_units, alloc);
		}

		const auto normalized = normalized_scalars(code_units, form);
		base_type result{ alloc };
		result.resize_and_overwrite(scalar_sequence_utf16_size(normalized),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (char32_t ch : normalized)
				{
					write_index += encode_unicode_scalar_utf16_unchecked(static_cast<std::uint32_t>(ch), buffer + write_index);
				}
				return write_index;
			});

		if (result.size() == code_units.size()
			&& std::char_traits<char16_t>::compare(result.data(), code_units.data(), code_units.size()) == 0)
		{
			return copy_utf16_view(code_units, alloc);
		}

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_utf8_owned(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>{ View::from_bytes_unchecked(byte_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_ascii_lowercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				ascii_lowercase_copy(buffer, bytes);
				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_ascii_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		const auto parts = details::checked_utf8_case_transform_parts(*this, pos, count);
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				const auto changed = ascii_lowercase_copy(buffer + parts.prefix.size(), parts.middle);
				std::char_traits<char8_t>::copy(
					buffer + parts.prefix.size() + parts.middle.size(),
					parts.suffix.data(),
					parts.suffix.size());

				if (!changed)
				{
					std::char_traits<char8_t>::copy(buffer + parts.prefix.size(), parts.middle.data(), parts.middle.size());
				}

				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_ascii_uppercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				ascii_uppercase_copy(buffer, bytes);
				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_ascii_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		const auto parts = details::checked_utf8_case_transform_parts(*this, pos, count);
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				const auto changed = ascii_uppercase_copy(buffer + parts.prefix.size(), parts.middle);
				std::char_traits<char8_t>::copy(
					buffer + parts.prefix.size() + parts.middle.size(),
					parts.suffix.data(),
					parts.suffix.size());

				if (!changed)
				{
					std::char_traits<char8_t>::copy(buffer + parts.prefix.size(), parts.middle.data(), parts.middle.size());
				}

				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_lowercase(const Allocator& alloc) const
	{
		return case_map_utf8_copy<true>(byte_view(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		const auto parts = details::checked_utf8_case_transform_parts(*this, pos, count);
		const auto measurement = details::measure_case_map_utf8<true>(parts.middle);
		if (!measurement.changed)
		{
			return copy_utf8_view(bytes, alloc);
		}

		const auto output_size = parts.prefix.size() + measurement.output_size + parts.suffix.size();
		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				auto write_index = parts.prefix.size();
				write_index += details::write_case_map_utf8_into<true>(parts.middle, buffer + write_index);
				std::char_traits<char8_t>::copy(buffer + write_index, parts.suffix.data(), parts.suffix.size());
				write_index += parts.suffix.size();
				return write_index;
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_lowercase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_lowercase_utf8_copy(byte_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_lowercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		const auto parts = details::checked_utf8_case_transform_parts(*this, pos, count);
		const auto lowered_middle = details::icu_lowercase_utf8_copy(parts.middle, locale, alloc);
		const auto lowered_bytes = std::u8string_view{ lowered_middle.base() };
		if (lowered_bytes.size() == parts.middle.size()
			&& std::char_traits<char8_t>::compare(lowered_bytes.data(), parts.middle.data(), parts.middle.size()) == 0)
		{
			return copy_utf8_view(bytes, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(parts.prefix.size() + lowered_bytes.size() + parts.suffix.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				std::char_traits<char8_t>::copy(buffer + parts.prefix.size(), lowered_bytes.data(), lowered_bytes.size());
				std::char_traits<char8_t>::copy(
					buffer + parts.prefix.size() + lowered_bytes.size(),
					parts.suffix.data(),
					parts.suffix.size());
				return parts.prefix.size() + lowered_bytes.size() + parts.suffix.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_uppercase(const Allocator& alloc) const
	{
		return case_map_utf8_copy<false>(byte_view(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		const auto parts = details::checked_utf8_case_transform_parts(*this, pos, count);
		const auto measurement = details::measure_case_map_utf8<false>(parts.middle);
		if (!measurement.changed)
		{
			return copy_utf8_view(bytes, alloc);
		}

		const auto output_size = parts.prefix.size() + measurement.output_size + parts.suffix.size();
		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				auto write_index = parts.prefix.size();
				write_index += details::write_case_map_utf8_into<false>(parts.middle, buffer + write_index);
				std::char_traits<char8_t>::copy(buffer + write_index, parts.suffix.data(), parts.suffix.size());
				write_index += parts.suffix.size();
				return write_index;
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_uppercase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_uppercase_utf8_copy(byte_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_uppercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		const auto parts = details::checked_utf8_case_transform_parts(*this, pos, count);
		const auto uppered_middle = details::icu_uppercase_utf8_copy(parts.middle, locale, alloc);
		const auto uppered_bytes = std::u8string_view{ uppered_middle.base() };
		if (uppered_bytes.size() == parts.middle.size()
			&& std::char_traits<char8_t>::compare(uppered_bytes.data(), parts.middle.data(), parts.middle.size()) == 0)
		{
			return copy_utf8_view(bytes, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(parts.prefix.size() + uppered_bytes.size() + parts.suffix.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char8_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				std::char_traits<char8_t>::copy(buffer + parts.prefix.size(), uppered_bytes.data(), uppered_bytes.size());
				std::char_traits<char8_t>::copy(
					buffer + parts.prefix.size() + uppered_bytes.size(),
					parts.suffix.data(),
					parts.suffix.size());
				return parts.prefix.size() + uppered_bytes.size() + parts.suffix.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::normalize(
		normalization_form form,
		const Allocator& alloc) const
	{
		return normalize_utf8_copy(byte_view(), form, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_nfc(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfc, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_nfd(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfd, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_nfkc(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfkc, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_nfkd(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfkd, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::case_fold(const Allocator& alloc) const
	{
		return case_fold_utf8_copy(byte_view(), alloc);
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
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, View to) const
	{
		return replace_all(View::from_bytes_unchecked(details::utf8_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(View from, utf8_char to) const
	{
		return replace_all(from, View::from_bytes_unchecked(details::utf8_char_view(to)));
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
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_bytes_unchecked(details::utf8_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(View from, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
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
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, utf8_char to) const
	{
		if (from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, View to) const
	{
		if (from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, utf8_char to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, View to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, utf8_char to) const
	{
		return replace_n(
			count,
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, View to) const
	{
		return replace_n(count, View::from_bytes_unchecked(details::utf8_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, utf8_char to) const
	{
		return replace_n(count, from, View::from_bytes_unchecked(details::utf8_char_view(to)));
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
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_bytes_unchecked(details::utf8_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
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
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, utf8_char to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, View to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, utf8_char to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, View to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(Pred pred, utf8_char to) const
	{
		return replace_all(std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(Pred pred, View to) const
	{
		return replace_all(std::move(pred), to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(Pred pred, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(Pred pred, View to, const Allocator& alloc) const
	{
		return replace_n(npos, std::move(pred), to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf8_char to) const
	{
		return replace_n(count, std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to) const
	{
		return replace_n(count, std::move(pred), to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(count, std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to, const Allocator& alloc) const
	{
		if (count == 0)
		{
			return to_utf8_owned(alloc);
		}

		const auto bytes = byte_view();
		const auto replacement = to.base();
		size_type replacements = 0;
		size_type replaced_size = 0;
		for (size_type pos = 0; pos < bytes.size() && replacements != count; )
		{
			const auto char_size = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[pos]));
			const auto ch = utf8_char::from_utf8_bytes_unchecked(bytes.data() + pos, char_size);
			if (static_cast<bool>(std::invoke(pred, ch)))
			{
				++replacements;
				replaced_size += char_size;
			}
			pos += char_size;
		}

		if (replacements == 0)
		{
			return to_utf8_owned(alloc);
		}

		using result_base = typename basic_utf8_string<Allocator>::base_type;
		result_base result{ alloc };
		const auto result_size = bytes.size() - replaced_size + replacements * replacement.size();
		result.resize_and_overwrite(result_size,
			[&](char8_t* buffer, std::size_t) constexpr
			{
				size_type read = 0;
				size_type write = 0;
				size_type replaced = 0;
				while (read < bytes.size())
				{
					const auto char_size = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read]));
					const auto ch = utf8_char::from_utf8_bytes_unchecked(bytes.data() + read, char_size);
					if (replaced != count && static_cast<bool>(std::invoke(pred, ch)))
					{
						if (!replacement.empty())
						{
							std::char_traits<char8_t>::copy(buffer + write, replacement.data(), replacement.size());
						}
						write += replacement.size();
						++replaced;
					}
					else
					{
						std::char_traits<char8_t>::copy(buffer + write, bytes.data() + read, char_size);
						write += char_size;
					}

					read += char_size;
				}

				return write;
			});
		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_utf16_owned(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>{ View::from_code_units_unchecked(code_unit_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_ascii_lowercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				ascii_lowercase_copy(buffer, code_units);
				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_uppercase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_uppercase_utf16_copy(code_unit_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_uppercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		const auto parts = details::checked_utf16_case_transform_parts(*this, pos, count);
		const auto uppered_middle = details::icu_uppercase_utf16_copy(parts.middle, locale, alloc);
		const auto uppered_code_units = std::u16string_view{ uppered_middle.base() };
		if (uppered_code_units.size() == parts.middle.size()
			&& std::char_traits<char16_t>::compare(uppered_code_units.data(), parts.middle.data(), parts.middle.size()) == 0)
		{
			return copy_utf16_view(code_units, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(parts.prefix.size() + uppered_code_units.size() + parts.suffix.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				std::char_traits<char16_t>::copy(buffer + parts.prefix.size(), uppered_code_units.data(), uppered_code_units.size());
				std::char_traits<char16_t>::copy(
					buffer + parts.prefix.size() + uppered_code_units.size(),
					parts.suffix.data(),
					parts.suffix.size());
				return parts.prefix.size() + uppered_code_units.size() + parts.suffix.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_ascii_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		const auto parts = details::checked_utf16_case_transform_parts(*this, pos, count);
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				const auto changed = ascii_lowercase_copy(buffer + parts.prefix.size(), parts.middle);
				std::char_traits<char16_t>::copy(
					buffer + parts.prefix.size() + parts.middle.size(),
					parts.suffix.data(),
					parts.suffix.size());

				if (!changed)
				{
					std::char_traits<char16_t>::copy(buffer + parts.prefix.size(), parts.middle.data(), parts.middle.size());
				}

				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_ascii_uppercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				ascii_uppercase_copy(buffer, code_units);
				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_ascii_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		const auto parts = details::checked_utf16_case_transform_parts(*this, pos, count);
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				const auto changed = ascii_uppercase_copy(buffer + parts.prefix.size(), parts.middle);
				std::char_traits<char16_t>::copy(
					buffer + parts.prefix.size() + parts.middle.size(),
					parts.suffix.data(),
					parts.suffix.size());

				if (!changed)
				{
					std::char_traits<char16_t>::copy(buffer + parts.prefix.size(), parts.middle.data(), parts.middle.size());
				}

				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_lowercase(const Allocator& alloc) const
	{
		return case_map_utf16_copy<true>(code_unit_view(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		const auto parts = details::checked_utf16_case_transform_parts(*this, pos, count);
		const auto measurement = details::measure_case_map_utf16<true>(parts.middle);
		if (!measurement.changed)
		{
			return copy_utf16_view(code_units, alloc);
		}

		const auto output_size = parts.prefix.size() + measurement.output_size + parts.suffix.size();
		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				auto write_index = parts.prefix.size();
				write_index += details::write_case_map_utf16_into<true>(parts.middle, buffer + write_index);
				std::char_traits<char16_t>::copy(buffer + write_index, parts.suffix.data(), parts.suffix.size());
				write_index += parts.suffix.size();
				return write_index;
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_lowercase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_lowercase_utf16_copy(code_unit_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_lowercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		const auto parts = details::checked_utf16_case_transform_parts(*this, pos, count);
		const auto lowered_middle = details::icu_lowercase_utf16_copy(parts.middle, locale, alloc);
		const auto lowered_code_units = std::u16string_view{ lowered_middle.base() };
		if (lowered_code_units.size() == parts.middle.size()
			&& std::char_traits<char16_t>::compare(lowered_code_units.data(), parts.middle.data(), parts.middle.size()) == 0)
		{
			return copy_utf16_view(code_units, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(parts.prefix.size() + lowered_code_units.size() + parts.suffix.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				std::char_traits<char16_t>::copy(buffer + parts.prefix.size(), lowered_code_units.data(), lowered_code_units.size());
				std::char_traits<char16_t>::copy(
					buffer + parts.prefix.size() + lowered_code_units.size(),
					parts.suffix.data(),
					parts.suffix.size());
				return parts.prefix.size() + lowered_code_units.size() + parts.suffix.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_uppercase(const Allocator& alloc) const
	{
		return case_map_utf16_copy<false>(code_unit_view(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		const auto parts = details::checked_utf16_case_transform_parts(*this, pos, count);
		const auto measurement = details::measure_case_map_utf16<false>(parts.middle);
		if (!measurement.changed)
		{
			return copy_utf16_view(code_units, alloc);
		}

		const auto output_size = parts.prefix.size() + measurement.output_size + parts.suffix.size();
		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char16_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				auto write_index = parts.prefix.size();
				write_index += details::write_case_map_utf16_into<false>(parts.middle, buffer + write_index);
				std::char_traits<char16_t>::copy(buffer + write_index, parts.suffix.data(), parts.suffix.size());
				write_index += parts.suffix.size();
				return write_index;
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::normalize(
		normalization_form form,
		const Allocator& alloc) const
	{
		return normalize_utf16_copy(code_unit_view(), form, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_nfc(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfc, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_nfd(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfd, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_nfkc(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfkc, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_nfkd(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfkd, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::case_fold(const Allocator& alloc) const
	{
		return case_fold_utf16_copy(code_unit_view(), alloc);
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
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, View to) const
	{
		return replace_all(View::from_code_units_unchecked(details::utf16_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(View from, utf16_char to) const
	{
		return replace_all(from, View::from_code_units_unchecked(details::utf16_char_view(to)));
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
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_code_units_unchecked(details::utf16_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(View from, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
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
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, utf16_char to) const
	{
		if (from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, View to) const
	{
		if (from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, utf16_char to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, View to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, utf16_char to) const
	{
		return replace_n(
			count,
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, View to) const
	{
		return replace_n(count, View::from_code_units_unchecked(details::utf16_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, utf16_char to) const
	{
		return replace_n(count, from, View::from_code_units_unchecked(details::utf16_char_view(to)));
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
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_code_units_unchecked(details::utf16_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
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

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, utf16_char to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, View to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, utf16_char to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, View to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(Pred pred, utf16_char to) const
	{
		return replace_all(std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(Pred pred, View to) const
	{
		return replace_all(std::move(pred), to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(Pred pred, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(Pred pred, View to, const Allocator& alloc) const
	{
		return replace_n(npos, std::move(pred), to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf16_char to) const
	{
		return replace_n(count, std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to) const
	{
		return replace_n(count, std::move(pred), to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(count, std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to, const Allocator& alloc) const
	{
		if (count == 0)
		{
			return to_utf16_owned(alloc);
		}

		const auto code_units = code_unit_view();
		const auto replacement = to.base();
		size_type replacements = 0;
		size_type replaced_size = 0;
		for (size_type pos = 0; pos < code_units.size() && replacements != count; )
		{
			const auto first = static_cast<std::uint16_t>(code_units[pos]);
			const auto char_size = details::is_utf16_high_surrogate(first) ? 2u : 1u;
			const auto ch = utf16_char::from_utf16_code_units_unchecked(code_units.data() + pos, char_size);
			if (static_cast<bool>(std::invoke(pred, ch)))
			{
				++replacements;
				replaced_size += char_size;
			}
			pos += char_size;
		}

		if (replacements == 0)
		{
			return to_utf16_owned(alloc);
		}

		using result_base = typename basic_utf16_string<Allocator>::base_type;
		result_base result{ alloc };
		const auto result_size = code_units.size() - replaced_size + replacements * replacement.size();
		result.resize_and_overwrite(result_size,
			[&](char16_t* buffer, std::size_t) constexpr
			{
				size_type read = 0;
				size_type write = 0;
				size_type replaced = 0;
				while (read < code_units.size())
				{
					const auto first = static_cast<std::uint16_t>(code_units[read]);
					const auto char_size = details::is_utf16_high_surrogate(first) ? 2u : 1u;
					const auto ch = utf16_char::from_utf16_code_units_unchecked(code_units.data() + read, char_size);
					if (replaced != count && static_cast<bool>(std::invoke(pred, ch)))
					{
						if (!replacement.empty())
						{
							std::char_traits<char16_t>::copy(buffer + write, replacement.data(), replacement.size());
						}
						write += replacement.size();
						++replaced;
					}
					else
					{
						std::char_traits<char16_t>::copy(buffer + write, code_units.data() + read, char_size);
						write += char_size;
					}

					read += char_size;
				}

				return write;
			});
		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_char::to_utf8_owned(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>{
			utf8_string_view::from_bytes_unchecked(details::utf8_char_view(*this)),
			alloc
		};
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_char::to_utf16_owned(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>{
			utf16_string_view::from_code_units_unchecked(details::utf16_char_view(*this)),
			alloc
		};
	}

}

#endif // UTF8_RANGES_TRANSCODING_HPP
