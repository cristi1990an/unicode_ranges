#if defined(UTF8_RANGES_UTF32_STRING_HPP) && defined(UTF8_RANGES_TRANSCODING_HPP) && !defined(UTF8_RANGES_TRANSCODING_UTF32_HPP)
#define UTF8_RANGES_TRANSCODING_UTF32_HPP
namespace unicode_ranges
{
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>::basic_utf8_string(utf32_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::operator+=(utf32_string_view sv)
	{
		return append_range(sv.chars());
	}
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>::basic_utf16_string(utf32_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::operator+=(utf32_string_view sv)
	{
		return append_range(sv.chars());
	}
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::append_range(views::utf32_view rg)
	{
		const auto code_points = rg.base();
		const auto appended_size = details::scalar_sequence_utf8_size(code_points);
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + appended_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				for (char32_t code_point : code_points)
				{
					write_index += details::encode_unicode_scalar_utf8_unchecked(
						static_cast<std::uint32_t>(code_point),
						buffer + write_index);
				}

				return write_index;
			});

		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::assign_range(views::utf32_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		const auto code_points = rg.base();
		replacement.resize_and_overwrite(details::scalar_sequence_utf8_size(code_points),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (char32_t code_point : code_points)
				{
					write_index += details::encode_unicode_scalar_utf8_unchecked(
						static_cast<std::uint32_t>(code_point),
						buffer + write_index);
				}

				return write_index;
			});
		base_ = std::move(replacement);
		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::append_range(views::utf32_view rg)
	{
		const auto code_points = rg.base();
		const auto appended_size = details::scalar_sequence_utf16_size(code_points);
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + appended_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				for (char32_t code_point : code_points)
				{
					write_index += details::encode_unicode_scalar_utf16_unchecked(
						static_cast<std::uint32_t>(code_point),
						buffer + write_index);
				}

				return write_index;
			});

		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::assign_range(views::utf32_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		const auto code_points = rg.base();
		replacement.resize_and_overwrite(details::scalar_sequence_utf16_size(code_points),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (char32_t code_point : code_points)
				{
					write_index += details::encode_unicode_scalar_utf16_unchecked(
						static_cast<std::uint32_t>(code_point),
						buffer + write_index);
				}

				return write_index;
			});
		base_ = std::move(replacement);
		return *this;
	}
	namespace details
	{
		inline constexpr std::size_t unicode_scalar_utf32_size(std::uint32_t) noexcept
		{
			return encoding_constants::single_code_unit_count;
		}
		template <typename Allocator>
		constexpr basic_utf32_string<Allocator> copy_utf32_view(
			std::u32string_view code_points,
			const Allocator& alloc)
		{
			using base_type = typename basic_utf32_string<Allocator>::base_type;
			base_type result{ alloc };
			result.resize_and_overwrite(code_points.size(),
				[&](char32_t* buffer, std::size_t) noexcept
				{
					std::char_traits<char32_t>::copy(buffer, code_points.data(), code_points.size());
					return code_points.size();
				});
			return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
		}
		template <bool Lowercase>
		constexpr case_map_measurement measure_case_map_utf32(std::u32string_view code_points) noexcept
		{
			case_map_measurement result{};
			for (std::size_t index = 0; index < code_points.size();)
			{
				const auto remaining = std::u32string_view{ code_points.data() + index, code_points.size() - index };
				const auto ascii_run = ascii_prefix_length(remaining);
				if (ascii_run != 0)
				{
					result.output_size += ascii_run;
					for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
					{
						const auto scalar = static_cast<std::uint32_t>(remaining[ascii_index]);
						result.changed = result.changed || (ascii_case_scalar<Lowercase>(scalar) != scalar);
					}
					index += ascii_run;
					continue;
				}
				const auto decoded = decode_next_scalar(code_points, index);
				if (decoded.scalar <= encoding_constants::bmp_scalar_max)
				{
					const auto bmp_mapping = lookup_bmp_case_mapping<Lowercase>(decoded.scalar);
					if (bmp_mapping.same_size)
					{
						result.changed = result.changed || (bmp_mapping.mapped != decoded.scalar);
						++result.output_size;
						index = decoded.next_index;
						continue;
					}
				}

				const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
				if (mapping.has_simple)
				{
					result.changed = true;
					result.output_size += unicode_scalar_utf32_size(mapping.simple_mapped);
				}
				else if (mapping.has_special())
				{
					result.changed = true;
					const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
					result.output_size += special.count;
				}
				else
				{
					result.output_size += decoded.next_index - index;
				}
				index = decoded.next_index;
			}
			return result;
		}
		struct utf32_case_transform_parts
		{
			std::u32string_view prefix;
			std::u32string_view middle;
			std::u32string_view suffix;
		};
		template <typename Derived, typename View>
		constexpr utf32_case_transform_parts checked_utf32_case_transform_parts(
			const utf32_string_crtp<Derived, View>& self,
			typename utf32_string_crtp<Derived, View>::size_type pos,
			typename utf32_string_crtp<Derived, View>::size_type count)
		{
			const auto code_points = std::u32string_view{ static_cast<const Derived&>(self).base() };
			if (pos > code_points.size()) [[unlikely]]
			{
				throw std::out_of_range("case transform index out of range");
			}
			const auto remaining = code_points.size() - pos;
			const auto transform_count = count == utf32_string_crtp<Derived, View>::npos ? remaining : count;
			if (transform_count > remaining) [[unlikely]]
			{
				throw std::out_of_range("case transform count out of range");
			}
			const auto end = pos + transform_count;
			if (!self.is_char_boundary(pos) || !self.is_char_boundary(end)) [[unlikely]]
			{
				throw std::out_of_range("case transform range must be a valid UTF-32 substring");
			}
			return utf32_case_transform_parts{
				code_points.substr(0, pos),
				code_points.substr(pos, transform_count),
				code_points.substr(end)
			};
		}
#if UTF8_RANGES_HAS_ICU
		template <typename Allocator>
		basic_utf32_string<Allocator> icu_lowercase_utf32_copy(
			std::u32string_view code_points,
			locale_id locale,
			const Allocator& alloc)
		{
			basic_utf16_string<> utf16;
			utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
			auto lowered = icu_lowercase_utf16_copy(std::u16string_view{ utf16.base() }, locale, std::allocator<char16_t>{});
			return basic_utf32_string<Allocator>{ lowered.as_view(), alloc };
		}
		template <typename Allocator>
		basic_utf32_string<Allocator> icu_uppercase_utf32_copy(
			std::u32string_view code_points,
			locale_id locale,
			const Allocator& alloc)
		{
			basic_utf16_string<> utf16;
			utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
			auto uppered = icu_uppercase_utf16_copy(std::u16string_view{ utf16.base() }, locale, std::allocator<char16_t>{});
			return basic_utf32_string<Allocator>{ uppered.as_view(), alloc };
		}
		template <typename Allocator>
		basic_utf32_string<Allocator> icu_titlecase_utf32_copy(
			std::u32string_view code_points,
			locale_id locale,
			const Allocator& alloc)
		{
			basic_utf16_string<> utf16;
			utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
			auto titled = icu_titlecase_utf16_copy(std::u16string_view{ utf16.base() }, locale, std::allocator<char16_t>{});
			return basic_utf32_string<Allocator>{ titled.as_view(), alloc };
		}
		template <typename Allocator>
		basic_utf32_string<Allocator> icu_case_fold_utf32_copy(
			std::u32string_view code_points,
			locale_id locale,
			const Allocator& alloc)
		{
			basic_utf16_string<> utf16;
			utf16.append_range(utf32_string_view::from_code_points_unchecked(code_points).chars());
			auto folded = icu_case_fold_utf16_copy(std::u16string_view{ utf16.base() }, locale, std::allocator<char16_t>{});
			return basic_utf32_string<Allocator>{ folded.as_view(), alloc };
		}
#endif
		template <bool Lowercase>
		constexpr std::size_t write_case_map_utf32_into(std::u32string_view code_points, char32_t* buffer) noexcept
		{
			std::size_t write_index = 0;
			for (std::size_t index = 0; index < code_points.size();)
			{
				const auto remaining = std::u32string_view{ code_points.data() + index, code_points.size() - index };
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
				const auto decoded = decode_next_scalar(code_points, index);
				if (decoded.scalar <= encoding_constants::bmp_scalar_max)
				{
					const auto bmp_mapping = lookup_bmp_case_mapping<Lowercase>(decoded.scalar);
					if (bmp_mapping.same_size)
					{
						buffer[write_index++] = static_cast<char32_t>(bmp_mapping.mapped);
						index = decoded.next_index;
						continue;
					}
				}

				const auto mapping = lookup_case_mapping<Lowercase>(decoded.scalar);
				if (mapping.has_simple)
				{
					buffer[write_index++] = static_cast<char32_t>(mapping.simple_mapped);
				}
				else if (mapping.has_special())
				{
					const auto& special = case_special_mapping_from_index<Lowercase>(mapping.special_index);
					for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
					{
						buffer[write_index++] = static_cast<char32_t>(special.mapped[mapped_index]);
					}
				}
				else
				{
					buffer[write_index++] = code_points[index];
				}
				index = decoded.next_index;
			}
			return write_index;
		}
		template <bool Lowercase, typename BaseType>
		constexpr void write_case_map_utf32(
			std::u32string_view code_points,
			BaseType& result,
			std::size_t output_size)
		{
			result.resize_and_overwrite(output_size,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					return write_case_map_utf32_into<Lowercase>(code_points, buffer);
				});
		}
		template <bool Lowercase, typename Allocator>
		constexpr basic_utf32_string<Allocator> case_map_utf32_copy(
			std::u32string_view code_points,
			const Allocator& alloc)
		{
			using base_type = typename basic_utf32_string<Allocator>::base_type;
			const auto measurement = measure_case_map_utf32<Lowercase>(code_points);
			if (!measurement.changed)
			{
				return copy_utf32_view(code_points, alloc);
			}
			base_type result{ alloc };
			write_case_map_utf32<Lowercase>(code_points, result, measurement.output_size);
			return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
		}
		constexpr case_map_measurement measure_case_fold_utf32(std::u32string_view code_points) noexcept
		{
			case_map_measurement result{};
			for (std::size_t index = 0; index < code_points.size();)
			{
				const auto remaining = std::u32string_view{ code_points.data() + index, code_points.size() - index };
				const auto ascii_run = ascii_prefix_length(remaining);
				if (ascii_run != 0)
				{
					result.output_size += ascii_run;
					for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
					{
						const auto scalar = static_cast<std::uint32_t>(remaining[ascii_index]);
						result.changed = result.changed || (lowercase_ascii_scalar(scalar) != scalar);
					}
					index += ascii_run;
					continue;
				}
				const auto decoded = decode_next_scalar(code_points, index);
				if (decoded.scalar <= encoding_constants::bmp_scalar_max)
				{
					const auto bmp_mapping = lookup_bmp_case_fold_mapping(decoded.scalar);
					if (bmp_mapping.same_size)
					{
						result.changed = result.changed || (bmp_mapping.mapped != decoded.scalar);
						++result.output_size;
						index = decoded.next_index;
						continue;
					}
				}

				const auto mapping = lookup_case_fold_mapping(decoded.scalar);
				if (mapping.has_simple)
				{
					result.changed = true;
					result.output_size += unicode_scalar_utf32_size(mapping.simple_mapped);
				}
				else if (mapping.has_special())
				{
					result.changed = true;
					const auto& special = case_fold_special_mapping_from_index(mapping.special_index);
					result.output_size += special.count;
				}
				else
				{
					result.output_size += decoded.next_index - index;
				}
				index = decoded.next_index;
			}
			return result;
		}
			constexpr std::size_t write_case_fold_utf32_into(std::u32string_view code_points, char32_t* buffer) noexcept
			{
				std::size_t write_index = 0;
			for (std::size_t index = 0; index < code_points.size();)
			{
				const auto remaining = std::u32string_view{ code_points.data() + index, code_points.size() - index };
				const auto ascii_run = ascii_prefix_length(remaining);
				if (ascii_run != 0)
				{
					ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
					write_index += ascii_run;
					index += ascii_run;
					continue;
				}
				const auto decoded = decode_next_scalar(code_points, index);
				if (decoded.scalar <= encoding_constants::bmp_scalar_max)
				{
					const auto bmp_mapping = lookup_bmp_case_fold_mapping(decoded.scalar);
					if (bmp_mapping.same_size)
					{
						buffer[write_index++] = static_cast<char32_t>(bmp_mapping.mapped);
						index = decoded.next_index;
						continue;
					}
				}

				const auto mapping = lookup_case_fold_mapping(decoded.scalar);
				if (mapping.has_simple)
				{
					buffer[write_index++] = static_cast<char32_t>(mapping.simple_mapped);
				}
				else if (mapping.has_special())
				{
					const auto& special = case_fold_special_mapping_from_index(mapping.special_index);
					for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
					{
						buffer[write_index++] = static_cast<char32_t>(special.mapped[mapped_index]);
					}
				}
				else
				{
					buffer[write_index++] = code_points[index];
				}
				index = decoded.next_index;
				}
				return write_index;
			}
			template <typename BaseType>
			constexpr bool try_case_fold_utf32_same_size(
				std::u32string_view code_points,
				BaseType& result,
				bool& changed) noexcept
			{
				bool same_size = true;
				changed = false;
				result.resize_and_overwrite(code_points.size(),
					[&](char32_t* buffer, std::size_t) noexcept
					{
						std::size_t write_index = 0;
						for (std::size_t index = 0; index < code_points.size();)
						{
							const auto remaining = std::u32string_view{ code_points.data() + index, code_points.size() - index };
							const auto ascii_run = ascii_prefix_length(remaining);
							if (ascii_run != 0)
							{
								changed = ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run)) || changed;
								write_index += ascii_run;
								index += ascii_run;
								continue;
							}

							const auto decoded = decode_next_scalar(code_points, index);
							const auto input_size = decoded.next_index - index;
							if (decoded.scalar <= encoding_constants::bmp_scalar_max)
							{
								const auto bmp_mapping = lookup_bmp_case_fold_mapping(decoded.scalar);
								if (bmp_mapping.same_size)
								{
									if (unicode_scalar_utf32_size(bmp_mapping.mapped) != input_size)
									{
										same_size = false;
										return write_index;
									}

									changed = changed || (bmp_mapping.mapped != decoded.scalar);
									buffer[write_index++] = static_cast<char32_t>(bmp_mapping.mapped);
									index = decoded.next_index;
									continue;
								}
							}

							const auto mapping = lookup_case_fold_mapping(decoded.scalar);
							if (mapping.has_special())
							{
								const auto& special = case_fold_special_mapping_from_index(mapping.special_index);
								if (special.count != input_size)
								{
									same_size = false;
									return write_index;
								}

								changed = true;
								for (std::size_t mapped_index = 0; mapped_index != special.count; ++mapped_index)
								{
									buffer[write_index++] = static_cast<char32_t>(special.mapped[mapped_index]);
								}
							}
							else if (mapping.has_simple)
							{
								if (unicode_scalar_utf32_size(mapping.simple_mapped) != input_size)
								{
									same_size = false;
									return write_index;
								}

								changed = true;
								buffer[write_index++] = static_cast<char32_t>(mapping.simple_mapped);
							}
							else
							{
								buffer[write_index++] = code_points[index];
							}

							index = decoded.next_index;
						}

						return write_index;
					});

				return same_size;
			}
			template <typename Allocator>
			constexpr basic_utf32_string<Allocator> case_fold_utf32_copy(
				std::u32string_view code_points,
				const Allocator& alloc)
			{
				using base_type = typename basic_utf32_string<Allocator>::base_type;
				base_type same_size_result{ alloc };
				bool changed = false;
				if (try_case_fold_utf32_same_size(code_points, same_size_result, changed))
				{
					if (!changed)
					{
						return copy_utf32_view(code_points, alloc);
					}

					return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(same_size_result));
				}

				const auto measurement = measure_case_fold_utf32(code_points);
				if (!measurement.changed)
				{
					return copy_utf32_view(code_points, alloc);
			}
			base_type result{ alloc };
			result.resize_and_overwrite(measurement.output_size,
				[&](char32_t* buffer, std::size_t) noexcept
				{
					return write_case_fold_utf32_into(code_points, buffer);
				});
			return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
		}
			class utf32_case_fold_reader
			{
			public:
				using code_unit_type = char32_t;

				constexpr explicit utf32_case_fold_reader(std::u32string_view code_points, bool turkic = false) noexcept
					: code_points_(code_points), turkic_(turkic)
				{
				}
				constexpr std::u32string_view ascii_run() const noexcept
				{
					if (mapping_index_ < mapping_.count || turkic_)
					{
						return {};
					}

					const auto remaining = std::u32string_view{ code_points_.data() + cursor_, code_points_.size() - cursor_ };
					return remaining.substr(0, ascii_prefix_length(remaining));
				}
				constexpr void skip_ascii_run(std::size_t count) noexcept
				{
					cursor_ += count;
				}
				constexpr bool next(std::uint32_t& scalar) noexcept
				{
					if (mapping_index_ < mapping_.count)
				{
					scalar = mapping_.data[mapping_index_++];
					return true;
				}
				if (cursor_ == code_points_.size())
				{
					return false;
				}
				const auto current = static_cast<std::uint32_t>(code_points_[cursor_]);
				if (current <= encoding_constants::ascii_scalar_max && !turkic_)
				{
					++cursor_;
					scalar = lowercase_ascii_scalar(current);
					return true;
				}
				++cursor_;
				mapping_.load(current, turkic_);
				mapping_index_ = 1;
				scalar = mapping_.data[0];
				return true;
			}
		private:
			std::u32string_view code_points_{};
			std::size_t cursor_ = 0;
			case_fold_scalar_buffer mapping_{};
			std::uint8_t mapping_index_ = 0;
			bool turkic_ = false;
		};
		class utf32_reverse_case_fold_reader
		{
		public:
			constexpr explicit utf32_reverse_case_fold_reader(std::u32string_view code_points, bool turkic = false) noexcept
				: code_points_(code_points), cursor_(code_points.size()), turkic_(turkic)
			{
			}
			constexpr bool next(std::uint32_t& scalar) noexcept
			{
				if (mapping_index_ != 0)
				{
					scalar = mapping_.data[--mapping_index_];
					return true;
				}
				if (cursor_ == 0)
				{
					return false;
				}
				const auto pos = cursor_ - 1u;
				const auto current = static_cast<std::uint32_t>(code_points_[pos]);
				cursor_ = pos;
				if (current <= encoding_constants::ascii_scalar_max && !turkic_)
				{
					scalar = lowercase_ascii_scalar(current);
					return true;
				}
				mapping_.load(current, turkic_);
				mapping_index_ = mapping_.count;
				scalar = mapping_.data[--mapping_index_];
				return true;
			}
		private:
			std::u32string_view code_points_{};
			std::size_t cursor_ = 0;
			case_fold_scalar_buffer mapping_{};
			std::uint8_t mapping_index_ = 0;
			bool turkic_ = false;
		};
			inline constexpr std::weak_ordering compare_case_folded_utf32(
				std::u32string_view lhs,
				std::u32string_view rhs) noexcept
			{
				return compare_case_folded_forward_sequences(
					utf32_case_fold_reader{ lhs },
					utf32_case_fold_reader{ rhs });
			}
			inline constexpr bool starts_with_case_folded_utf32(
				std::u32string_view text,
				std::u32string_view prefix) noexcept
			{
				return folded_forward_sequence_starts_with(
					utf32_case_fold_reader{ text },
					utf32_case_fold_reader{ prefix });
			}
		inline constexpr bool ends_with_case_folded_utf32(
			std::u32string_view text,
			std::u32string_view suffix) noexcept
		{
			return folded_sequence_starts_with(
				utf32_reverse_case_fold_reader{ text },
				utf32_reverse_case_fold_reader{ suffix });
		}
#if UTF8_RANGES_HAS_ICU
			inline std::weak_ordering compare_case_folded_utf32(
				std::u32string_view lhs,
				std::u32string_view rhs,
				locale_id locale)
			{
				const auto turkic = icu_case_fold_is_turkic(locale);
				return compare_case_folded_forward_sequences(
					utf32_case_fold_reader{ lhs, turkic },
					utf32_case_fold_reader{ rhs, turkic });
			}
			inline bool starts_with_case_folded_utf32(
				std::u32string_view text,
				std::u32string_view prefix,
				locale_id locale)
			{
				const auto turkic = icu_case_fold_is_turkic(locale);
				return folded_forward_sequence_starts_with(
					utf32_case_fold_reader{ text, turkic },
					utf32_case_fold_reader{ prefix, turkic });
			}
		inline bool ends_with_case_folded_utf32(
			std::u32string_view text,
			std::u32string_view suffix,
			locale_id locale)
		{
			const auto turkic = icu_case_fold_is_turkic(locale);
			return folded_sequence_starts_with(
				utf32_reverse_case_fold_reader{ text, turkic },
				utf32_reverse_case_fold_reader{ suffix, turkic });
		}
#endif
		template <typename Allocator>
		constexpr basic_utf32_string<Allocator> normalize_utf32_copy(
			std::u32string_view code_points,
			normalization_form form,
			const Allocator& alloc)
		{
			using base_type = typename basic_utf32_string<Allocator>::base_type;
			if (code_points.empty())
			{
				return basic_utf32_string<Allocator>::from_code_points_unchecked(base_type{ alloc });
			}
			if (is_ascii_only(code_points))
			{
				return copy_utf32_view(code_points, alloc);
			}
			if (form == normalization_form::nfc && nfc_quick_check_pass(code_points))
			{
				return copy_utf32_view(code_points, alloc);
			}
			const auto normalized = normalized_scalars(code_points, form);
			if (normalized.size() == code_points.size()
				&& std::char_traits<char32_t>::compare(normalized.data(), code_points.data(), code_points.size()) == 0)
			{
				return copy_utf32_view(code_points, alloc);
			}
			base_type result{ alloc };
			result.resize_and_overwrite(normalized.size(),
				[&](char32_t* buffer, std::size_t) noexcept
				{
					std::char_traits<char32_t>::copy(buffer, normalized.data(), normalized.size());
					return normalized.size();
				});
			return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
		}
	}

	namespace details
	{
	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf8_string_crtp<Derived, View>::to_utf32(const Allocator& alloc) const
	{
		basic_utf32_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}
	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf16_string_crtp<Derived, View>::to_utf32(const Allocator& alloc) const
	{
		basic_utf32_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}
	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_utf32_owned(const Allocator& alloc) const
	{
		return basic_utf32_string<Allocator>{ View::from_code_points_unchecked(code_unit_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_ascii_lowercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		base_type result{ alloc };
		result.resize_and_overwrite(code_points.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				ascii_lowercase_copy(buffer, code_points);
				return code_points.size();
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_uppercase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_uppercase_utf32_copy(code_unit_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_uppercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		const auto parts = details::checked_utf32_case_transform_parts(*this, pos, count);
		const auto uppered_middle = details::icu_uppercase_utf32_copy(parts.middle, locale, alloc);
		const auto uppered_code_points = std::u32string_view{ uppered_middle.base() };
		if (uppered_code_points.size() == parts.middle.size()
			&& std::char_traits<char32_t>::compare(uppered_code_points.data(), parts.middle.data(), parts.middle.size()) == 0)
		{
			return details::copy_utf32_view(code_points, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(parts.prefix.size() + uppered_code_points.size() + parts.suffix.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				std::char_traits<char32_t>::copy(buffer + parts.prefix.size(), uppered_code_points.data(), uppered_code_points.size());
				std::char_traits<char32_t>::copy(
					buffer + parts.prefix.size() + uppered_code_points.size(),
					parts.suffix.data(),
					parts.suffix.size());
				return parts.prefix.size() + uppered_code_points.size() + parts.suffix.size();
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_titlecase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_titlecase_utf32_copy(code_unit_view(), locale, alloc);
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_ascii_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		const auto parts = details::checked_utf32_case_transform_parts(*this, pos, count);
		base_type result{ alloc };
		result.resize_and_overwrite(code_points.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				const auto changed = ascii_lowercase_copy(buffer + parts.prefix.size(), parts.middle);
				std::char_traits<char32_t>::copy(
					buffer + parts.prefix.size() + parts.middle.size(),
					parts.suffix.data(),
					parts.suffix.size());

				if (!changed)
				{
					std::char_traits<char32_t>::copy(buffer + parts.prefix.size(), parts.middle.data(), parts.middle.size());
				}

				return code_points.size();
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_ascii_uppercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		base_type result{ alloc };
		result.resize_and_overwrite(code_points.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				ascii_uppercase_copy(buffer, code_points);
				return code_points.size();
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_ascii_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		const auto parts = details::checked_utf32_case_transform_parts(*this, pos, count);
		base_type result{ alloc };
		result.resize_and_overwrite(code_points.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				const auto changed = ascii_uppercase_copy(buffer + parts.prefix.size(), parts.middle);
				std::char_traits<char32_t>::copy(
					buffer + parts.prefix.size() + parts.middle.size(),
					parts.suffix.data(),
					parts.suffix.size());

				if (!changed)
				{
					std::char_traits<char32_t>::copy(buffer + parts.prefix.size(), parts.middle.data(), parts.middle.size());
				}

				return code_points.size();
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_lowercase(const Allocator& alloc) const
	{
		return details::case_map_utf32_copy<true>(code_unit_view(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_lowercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		const auto parts = details::checked_utf32_case_transform_parts(*this, pos, count);
		const auto measurement = details::measure_case_map_utf32<true>(parts.middle);
		if (!measurement.changed)
		{
			return details::copy_utf32_view(code_points, alloc);
		}

		const auto output_size = parts.prefix.size() + measurement.output_size + parts.suffix.size();
		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				auto write_index = parts.prefix.size();
				write_index += details::write_case_map_utf32_into<true>(parts.middle, buffer + write_index);
				std::char_traits<char32_t>::copy(buffer + write_index, parts.suffix.data(), parts.suffix.size());
				write_index += parts.suffix.size();
				return write_index;
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_lowercase(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_lowercase_utf32_copy(code_unit_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_lowercase(
		size_type pos,
		size_type count,
		locale_id locale,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		const auto parts = details::checked_utf32_case_transform_parts(*this, pos, count);
		const auto lowered_middle = details::icu_lowercase_utf32_copy(parts.middle, locale, alloc);
		const auto lowered_code_points = std::u32string_view{ lowered_middle.base() };
		if (lowered_code_points.size() == parts.middle.size()
			&& std::char_traits<char32_t>::compare(lowered_code_points.data(), parts.middle.data(), parts.middle.size()) == 0)
		{
			return details::copy_utf32_view(code_points, alloc);
		}

		base_type result{ alloc };
		result.resize_and_overwrite(parts.prefix.size() + lowered_code_points.size() + parts.suffix.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				std::char_traits<char32_t>::copy(buffer + parts.prefix.size(), lowered_code_points.data(), lowered_code_points.size());
				std::char_traits<char32_t>::copy(
					buffer + parts.prefix.size() + lowered_code_points.size(),
					parts.suffix.data(),
					parts.suffix.size());
				return parts.prefix.size() + lowered_code_points.size() + parts.suffix.size();
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_uppercase(const Allocator& alloc) const
	{
		return details::case_map_utf32_copy<false>(code_unit_view(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_uppercase(
		size_type pos,
		size_type count,
		const Allocator& alloc) const
	{
		using base_type = typename basic_utf32_string<Allocator>::base_type;
		const auto code_points = code_unit_view();
		const auto parts = details::checked_utf32_case_transform_parts(*this, pos, count);
		const auto measurement = details::measure_case_map_utf32<false>(parts.middle);
		if (!measurement.changed)
		{
			return details::copy_utf32_view(code_points, alloc);
		}

		const auto output_size = parts.prefix.size() + measurement.output_size + parts.suffix.size();
		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::char_traits<char32_t>::copy(buffer, parts.prefix.data(), parts.prefix.size());
				auto write_index = parts.prefix.size();
				write_index += details::write_case_map_utf32_into<false>(parts.middle, buffer + write_index);
				std::char_traits<char32_t>::copy(buffer + write_index, parts.suffix.data(), parts.suffix.size());
				write_index += parts.suffix.size();
				return write_index;
			});

		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::normalize(
		normalization_form form,
		const Allocator& alloc) const
	{
		return details::normalize_utf32_copy(code_unit_view(), form, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_nfc(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfc, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_nfd(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfd, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_nfkc(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfkc, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::to_nfkd(const Allocator& alloc) const
	{
		return normalize(normalization_form::nfkd, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::case_fold(const Allocator& alloc) const
	{
		return case_fold_utf32_copy(code_unit_view(), alloc);
	}

#if UTF8_RANGES_HAS_ICU
	template <typename Derived, typename View>
	template <typename Allocator>
	basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::case_fold(locale_id locale, const Allocator& alloc) const
	{
		return details::icu_case_fold_utf32_copy(code_unit_view(), locale, alloc);
	}

	template <typename Derived, typename View>
	bool utf32_string_crtp<Derived, View>::eq_ignore_case(View sv, locale_id locale) const
	{
		return compare_ignore_case(sv, locale) == std::weak_ordering::equivalent;
	}

	template <typename Derived, typename View>
	bool utf32_string_crtp<Derived, View>::starts_with_ignore_case(View sv, locale_id locale) const
	{
		const auto lhs = code_unit_view();
		const auto rhs = sv.base();
		const auto turkic = details::icu_case_fold_is_turkic(locale);
		if (!turkic && details::is_ascii_only(lhs) && details::is_ascii_only(rhs))
		{
			return details::starts_with_ascii_case_insensitive(lhs, rhs);
		}

		return details::starts_with_case_folded_utf32(lhs, rhs, locale);
	}

	template <typename Derived, typename View>
	bool utf32_string_crtp<Derived, View>::ends_with_ignore_case(View sv, locale_id locale) const
	{
		const auto lhs = code_unit_view();
		const auto rhs = sv.base();
		const auto turkic = details::icu_case_fold_is_turkic(locale);
		if (!turkic && details::is_ascii_only(lhs) && details::is_ascii_only(rhs))
		{
			return details::ends_with_ascii_case_insensitive(lhs, rhs);
		}

		return details::ends_with_case_folded_utf32(lhs, rhs, locale);
	}

	template <typename Derived, typename View>
	std::weak_ordering utf32_string_crtp<Derived, View>::compare_ignore_case(View sv, locale_id locale) const
	{
		const auto lhs = code_unit_view();
		const auto rhs = sv.base();
		const auto turkic = details::icu_case_fold_is_turkic(locale);
		if (!turkic && details::is_ascii_only(lhs) && details::is_ascii_only(rhs))
		{
			return details::compare_ascii_case_insensitive(lhs, rhs);
		}

		return details::compare_case_folded_utf32(lhs, rhs, locale);
	}
#endif

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf32_string_crtp<Derived, View>::to_utf8(const Allocator& alloc) const
	{
		basic_utf8_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}

	template <typename Derived, typename View>
	constexpr bool utf32_string_crtp<Derived, View>::eq_ignore_case(View sv) const noexcept
	{
		return compare_ignore_case(sv) == std::weak_ordering::equivalent;
	}

	template <typename Derived, typename View>
	constexpr bool utf32_string_crtp<Derived, View>::starts_with_ignore_case(View sv) const noexcept
	{
		const auto lhs = code_unit_view();
		const auto rhs = sv.base();
		if (details::is_ascii_only(lhs) && details::is_ascii_only(rhs))
		{
			return details::starts_with_ascii_case_insensitive(lhs, rhs);
		}

		return details::starts_with_case_folded_utf32(lhs, rhs);
	}

	template <typename Derived, typename View>
	constexpr bool utf32_string_crtp<Derived, View>::ends_with_ignore_case(View sv) const noexcept
	{
		const auto lhs = code_unit_view();
		const auto rhs = sv.base();
		if (details::is_ascii_only(lhs) && details::is_ascii_only(rhs))
		{
			return details::ends_with_ascii_case_insensitive(lhs, rhs);
		}

		return details::ends_with_case_folded_utf32(lhs, rhs);
	}

	template <typename Derived, typename View>
	constexpr std::weak_ordering utf32_string_crtp<Derived, View>::compare_ignore_case(View sv) const noexcept
	{
		const auto lhs = code_unit_view();
		const auto rhs = sv.base();
		if (details::is_ascii_only(lhs) && details::is_ascii_only(rhs))
		{
			return details::compare_ascii_case_insensitive(lhs, rhs);
		}

		return details::compare_case_folded_utf32(lhs, rhs);
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(utf32_char from, utf32_char to) const
	{
		return replace_all(
			View::from_code_points_unchecked(details::utf32_char_view(from)),
			View::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(utf32_char from, View to) const
	{
		return replace_all(View::from_code_points_unchecked(details::utf32_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(View from, utf32_char to) const
	{
		return replace_all(from, View::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(View from, View to) const
	{
		return replace_all(from, to, std::allocator<char32_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(utf32_char from, utf32_char to, const Allocator& alloc) const
	{
		return replace_all(
			View::from_code_points_unchecked(details::utf32_char_view(from)),
			View::from_code_points_unchecked(details::utf32_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(utf32_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_code_points_unchecked(details::utf32_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(View from, utf32_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_code_points_unchecked(details::utf32_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(View from, View to, const Allocator& alloc) const
	{
		return basic_utf32_string<Allocator>::from_code_points_unchecked(details::replace_utf32_code_points_copy(
			code_unit_view(),
			from.base(),
			to.base(),
			npos,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(std::span<const utf32_char> from, utf32_char to) const
	{
		if (from.empty())
		{
			return to_utf32_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf32_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(std::span<const utf32_char> from, View to) const
	{
		if (from.empty())
		{
			return to_utf32_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf32_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(std::span<const utf32_char> from, utf32_char to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf32_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf32_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(std::span<const utf32_char> from, View to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf32_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf32_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, utf32_char from, utf32_char to) const
	{
		return replace_n(
			count,
			View::from_code_points_unchecked(details::utf32_char_view(from)),
			View::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, utf32_char from, View to) const
	{
		return replace_n(count, View::from_code_points_unchecked(details::utf32_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, View from, utf32_char to) const
	{
		return replace_n(count, from, View::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, View from, View to) const
	{
		return replace_n(count, from, to, std::allocator<char32_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, utf32_char from, utf32_char to, const Allocator& alloc) const
	{
		return replace_n(
			count,
			View::from_code_points_unchecked(details::utf32_char_view(from)),
			View::from_code_points_unchecked(details::utf32_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, utf32_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_code_points_unchecked(details::utf32_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, View from, utf32_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_code_points_unchecked(details::utf32_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, View from, View to, const Allocator& alloc) const
	{
		return basic_utf32_string<Allocator>::from_code_points_unchecked(details::replace_utf32_code_points_copy(
			code_unit_view(),
			from.base(),
			to.base(),
			count,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf32_char> from, utf32_char to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf32_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf32_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf32_char> from, View to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf32_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf32_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf32_char> from, utf32_char to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf32_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf32_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf32_char> from, View to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf32_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf32_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(Pred pred, utf32_char to) const
	{
		return replace_all(std::move(pred), View::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_all(Pred pred, View to) const
	{
		return replace_all(std::move(pred), to, std::allocator<char32_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred, typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(Pred pred, utf32_char to, const Allocator& alloc) const
	{
		return replace_all(std::move(pred), View::from_code_points_unchecked(details::utf32_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred, typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_all(Pred pred, View to, const Allocator& alloc) const
	{
		return replace_n(npos, std::move(pred), to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf32_char to) const
	{
		return replace_n(count, std::move(pred), View::from_code_points_unchecked(details::utf32_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred>
	constexpr basic_utf32_string<> utf32_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to) const
	{
		return replace_n(count, std::move(pred), to, std::allocator<char32_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred, typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf32_char to, const Allocator& alloc) const
	{
		return replace_n(count, std::move(pred), View::from_code_points_unchecked(details::utf32_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf32_char_predicate Pred, typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to, const Allocator& alloc) const
	{
		if (count == 0)
		{
			return to_utf32_owned(alloc);
		}

		const auto code_points = code_unit_view();
		const auto replacement = to.base();
		size_type replacements = 0;
		size_type replaced_size = 0;
		for (size_type pos = 0; pos < code_points.size() && replacements != count; )
		{
			const auto char_size = details::encoding_constants::single_code_unit_count;
			const auto ch = utf32_char::from_utf32_code_points_unchecked(code_points.data() + pos, char_size);
			if (static_cast<bool>(std::invoke(pred, ch)))
			{
				++replacements;
				replaced_size += char_size;
			}
			pos += char_size;
		}

		if (replacements == 0)
		{
			return to_utf32_owned(alloc);
		}

		using result_base = typename basic_utf32_string<Allocator>::base_type;
		result_base result{ alloc };
		const auto result_size = code_points.size() - replaced_size + replacements * replacement.size();
		result.resize_and_overwrite(result_size,
			[&](char32_t* buffer, std::size_t) constexpr
			{
				size_type read = 0;
				size_type write = 0;
				size_type replaced = 0;
				while (read < code_points.size())
				{
					const auto char_size = details::encoding_constants::single_code_unit_count;
					const auto ch = utf32_char::from_utf32_code_points_unchecked(code_points.data() + read, char_size);
					if (replaced != count && static_cast<bool>(std::invoke(pred, ch)))
					{
						if (!replacement.empty())
						{
							std::char_traits<char32_t>::copy(buffer + write, replacement.data(), replacement.size());
						}
						write += replacement.size();
						++replaced;
					}
					else
					{
						std::char_traits<char32_t>::copy(buffer + write, code_points.data() + read, char_size);
						write += char_size;
					}

					read += char_size;
				}

				return write;
			});
		return basic_utf32_string<Allocator>::from_code_points_unchecked(std::move(result));
	}
	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf32_string_crtp<Derived, View>::to_utf16(const Allocator& alloc) const
	{
		basic_utf16_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}
	}

	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>::basic_utf32_string(utf8_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>::basic_utf32_string(utf16_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::append_range(views::utf32_view rg)
	{
		return append_code_points(rg.base());
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::append_range(views::utf8_view rg)
	{
		const auto bytes = rg.base();
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + bytes.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
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
					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					buffer[write_index++] = static_cast<char32_t>(details::decode_valid_utf8_char(bytes.data() + read_index, count));
					read_index += count;
				}
				return write_index;
			});
		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::append_range(views::utf16_view rg)
	{
		const auto code_units = rg.base();
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + code_units.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto remaining = std::u16string_view{ code_units.data() + read_index, code_units.size() - read_index };
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
					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					buffer[write_index++] = static_cast<char32_t>(details::decode_valid_utf16_char(code_units.data() + read_index, count));
					read_index += count;
				}
				return write_index;
			});
		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::assign_range(views::utf32_view rg)
	{
		const auto code_points = rg.base();
		if (overlaps_base(code_points))
		{
			base_type replacement{ code_points, base_.get_allocator() };
			base_ = std::move(replacement);
		}
		else
		{
			base_.assign(code_points);
		}
		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::assign_range(views::utf8_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		const auto bytes = rg.base();
		replacement.resize_and_overwrite(bytes.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
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
					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					buffer[write_index++] = static_cast<char32_t>(details::decode_valid_utf8_char(bytes.data() + read_index, count));
					read_index += count;
				}
				return write_index;
			});
		base_ = std::move(replacement);
		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::assign_range(views::utf16_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		const auto code_units = rg.base();
		replacement.resize_and_overwrite(code_units.size(),
			[&](char32_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto remaining = std::u16string_view{ code_units.data() + read_index, code_units.size() - read_index };
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
					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					buffer[write_index++] = static_cast<char32_t>(details::decode_valid_utf16_char(code_units.data() + read_index, count));
					read_index += count;
				}
				return write_index;
			});
		base_ = std::move(replacement);
		return *this;
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::operator+=(utf8_string_view sv)
	{
		return append_range(sv.chars());
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator>& basic_utf32_string<Allocator>::operator+=(utf16_string_view sv)
	{
		return append_range(sv.chars());
	}
	template <typename Allocator>
	constexpr basic_utf32_string<Allocator> utf32_char::to_utf32_owned(const Allocator& alloc) const
	{
		return basic_utf32_string<Allocator>{
			utf32_string_view::from_code_points_unchecked(details::utf32_char_view(*this)),
			alloc };
	}
}
#endif // UTF8_RANGES_TRANSCODING_UTF32_HPP
