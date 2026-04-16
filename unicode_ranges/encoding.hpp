#ifndef UTF8_RANGES_ENCODING_HPP
#define UTF8_RANGES_ENCODING_HPP

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <memory>
#include <optional>
#include <span>
#include <typeinfo>

#include "utf8_string_view.hpp"
#include "utf16_string_view.hpp"
#include "utf32_string_view.hpp"

#ifndef UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS
#ifdef NDEBUG
#define UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS 0
#else
#define UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS 1
#endif
#endif

namespace unicode_ranges
{

struct codec_contract_violation : std::logic_error
{
	using std::logic_error::logic_error;
};

template <typename Encoder, typename OutputAllocator = std::allocator<typename Encoder::code_unit_type>>
struct encoded_string_type
{
	using type = std::basic_string<
		typename Encoder::code_unit_type,
		std::char_traits<typename Encoder::code_unit_type>,
		OutputAllocator>;
};

template <typename Encoder>
struct encoder_traits;

template <typename Decoder>
struct decoder_traits;

template <typename Decoder, typename UtfString>
using from_encoded_result = std::conditional_t<
	std::same_as<typename decoder_traits<Decoder>::decode_error, void>,
	UtfString,
	std::expected<UtfString, typename decoder_traits<Decoder>::decode_error>>;

template <typename Encoder, typename OutputAllocator = std::allocator<typename encoder_traits<Encoder>::code_unit_type>>
using to_encoded_result = std::conditional_t<
	std::same_as<typename encoder_traits<Encoder>::encode_error, void>,
	std::basic_string<
		typename encoder_traits<Encoder>::code_unit_type,
		std::char_traits<typename encoder_traits<Encoder>::code_unit_type>,
		OutputAllocator>,
	std::expected<
		std::basic_string<
			typename encoder_traits<Encoder>::code_unit_type,
			std::char_traits<typename encoder_traits<Encoder>::code_unit_type>,
			OutputAllocator>,
		typename encoder_traits<Encoder>::encode_error>>;

enum class encode_to_error_kind
{
	overflow,
	encoding_error
};

template <typename Encoder, bool = !std::same_as<typename encoder_traits<Encoder>::encode_error, void>>
struct encode_to_error;

template <typename Encoder>
struct encode_to_error<Encoder, false>
{
	encode_to_error_kind kind = encode_to_error_kind::overflow;

	static constexpr encode_to_error overflow() noexcept
	{
		return {};
	}
};

template <typename Encoder>
struct encode_to_error<Encoder, true>
{
	using encoding_error_type = typename encoder_traits<Encoder>::encode_error;

	encode_to_error_kind kind = encode_to_error_kind::overflow;
	std::optional<encoding_error_type> error{};

	static constexpr encode_to_error overflow() noexcept
	{
		return {};
	}

	static constexpr encode_to_error encoding(encoding_error_type error_value)
	{
		return encode_to_error{
			.kind = encode_to_error_kind::encoding_error,
			.error = std::move(error_value)
		};
	}
};

namespace details
{

template <typename T, typename Default, template <typename> class Alias>
struct detected_or
{
	using type = Default;
};

template <typename T, typename Default, template <typename> class Alias>
	requires requires { typename Alias<T>; }
struct detected_or<T, Default, Alias>
{
	using type = Alias<T>;
};

template <typename T, template <typename> class Alias>
using detected_t = typename detected_or<T, void, Alias>::type;

template <typename T>
using codec_code_unit_type_t = typename T::code_unit_type;

template <typename T>
using codec_encode_error_t = typename T::encode_error;

template <typename T>
using codec_decode_error_t = typename T::decode_error;

template <typename T>
inline constexpr bool has_explicit_allow_implicit_construction_v =
	requires { T::allow_implicit_construction; };

template <typename T>
consteval bool explicit_allow_implicit_construction_value()
{
	if constexpr (has_explicit_allow_implicit_construction_v<T>)
	{
		return static_cast<bool>(T::allow_implicit_construction);
	}
	else
	{
		return false;
	}
}

template <typename T>
inline constexpr bool explicit_allow_implicit_construction_v =
	explicit_allow_implicit_construction_value<T>();

template <typename T>
inline constexpr bool fallback_allow_implicit_construction_v =
	std::is_empty_v<T> && std::default_initializable<T>;

template <typename T>
consteval bool allow_implicit_construction_requested_value()
{
	if constexpr (has_explicit_allow_implicit_construction_v<T>)
	{
		return explicit_allow_implicit_construction_v<T>;
	}
	else
	{
		return fallback_allow_implicit_construction_v<T>;
	}
}

template <typename T>
inline constexpr bool allow_implicit_construction_requested_v =
	allow_implicit_construction_requested_value<T>();

	template <typename Unit>
	struct probe_code_unit_writer
	{
		using unit_type = Unit;

		constexpr void reserve(std::size_t) noexcept
		{
		}

		constexpr void push(Unit) noexcept
		{
		}

		constexpr void append(std::span<const Unit>) noexcept
		{
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, Unit>
		constexpr void append(R&&) noexcept
		{
		}
	};

	struct probe_scalar_writer
	{
		using unit_type = char32_t;

		constexpr void reserve(std::size_t) noexcept
		{
		}

		constexpr void push(char32_t) noexcept
		{
		}

		constexpr void append(std::span<const char32_t>) noexcept
		{
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, char32_t>
		constexpr void append(R&&) noexcept
		{
		}
	};

	template <typename Error>
	[[noreturn]]
	inline void on_codec_contract_violation(const char* message)
	{
#if UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS
#if defined(__cpp_exceptions) || defined(_CPPUNWIND)
		throw codec_contract_violation(message);
#else
		(void)message;
		std::terminate();
#endif
#else
		(void)message;
		std::terminate();
#endif
	}

	inline constexpr void validate_codec_scalar(char32_t scalar)
	{
#if UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS
		if (!details::is_valid_unicode_scalar(static_cast<std::uint32_t>(scalar))) [[unlikely]]
		{
			on_codec_contract_violation<void>("codec emitted an invalid Unicode scalar");
		}
#else
		(void)scalar;
#endif
	}

	inline constexpr void validate_decode_progress(std::size_t consumed, std::size_t available)
	{
#if UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS
		if (consumed == 0 || consumed > available) [[unlikely]]
		{
			on_codec_contract_violation<void>("decoder reported invalid consumed input");
		}
#else
		(void)consumed;
		(void)available;
#endif
	}

	template <typename Unit, typename Iterator, typename Sentinel>
	struct bounded_output_state
	{
		Iterator current;
		Sentinel end;
		bool overflowed = false;
	};

	template <
		typename Unit,
		typename Iterator,
		typename Sentinel,
		bool = std::contiguous_iterator<Iterator>
			&& std::sized_sentinel_for<Sentinel, Iterator>
			&& std::same_as<std::remove_cv_t<std::iter_value_t<Iterator>>, Unit>>
	class bounded_output_writer
	{
	public:
		using unit_type = Unit;

		constexpr explicit bounded_output_writer(bounded_output_state<Unit, Iterator, Sentinel>& state) noexcept
			: state_(&state)
		{
		}

		constexpr void reserve(std::size_t) const noexcept
		{
		}

		constexpr void push(Unit unit) const
		{
			if (state_->overflowed) [[unlikely]]
			{
				return;
			}

			if (state_->current == state_->end) [[unlikely]]
			{
				state_->overflowed = true;
				return;
			}

			*state_->current = unit;
			++state_->current;
		}

		constexpr void append(std::span<const Unit> units) const
		{
			for (const Unit unit : units)
			{
				push(unit);
			}
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, Unit>
		constexpr void append(R&& units) const
		{
			if constexpr (
				std::ranges::contiguous_range<R>
				&& std::ranges::sized_range<R>
				&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, Unit>)
			{
				append(std::span<const Unit>{ std::ranges::data(units), std::ranges::size(units) });
			}
			else
			{
				for (auto&& unit : units)
				{
					push(static_cast<Unit>(unit));
				}
			}
		}

		[[nodiscard]]
		constexpr bool overflowed() const noexcept
		{
			return state_->overflowed;
		}

	private:
		bounded_output_state<Unit, Iterator, Sentinel>* state_ = nullptr;
	};

	template <typename Unit, typename Iterator, typename Sentinel>
	class bounded_output_writer<Unit, Iterator, Sentinel, true>
	{
	public:
		using unit_type = Unit;

		constexpr explicit bounded_output_writer(bounded_output_state<Unit, Iterator, Sentinel>& state) noexcept
			: state_(&state)
		{
		}

		constexpr void reserve(std::size_t) const noexcept
		{
		}

		constexpr void push(Unit unit) const
		{
			if (state_->overflowed) [[unlikely]]
			{
				return;
			}

			if (state_->current == state_->end) [[unlikely]]
			{
				state_->overflowed = true;
				return;
			}

			*state_->current = unit;
			++state_->current;
		}

		constexpr void append(std::span<const Unit> units) const
		{
			if (state_->overflowed) [[unlikely]]
			{
				return;
			}

			if (units.empty()) [[unlikely]]
			{
				return;
			}

			const auto available = static_cast<std::size_t>(state_->end - state_->current);
			const auto to_write = (std::min)(available, units.size());

			if (to_write != 0)
			{
				std::char_traits<Unit>::copy(std::to_address(state_->current), units.data(), to_write);
				state_->current += static_cast<typename std::iterator_traits<Iterator>::difference_type>(to_write);
			}

			if (to_write != units.size()) [[unlikely]]
			{
				state_->overflowed = true;
			}
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, Unit>
		constexpr void append(R&& units) const
		{
			if constexpr (
				std::ranges::contiguous_range<R>
				&& std::ranges::sized_range<R>
				&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, Unit>)
			{
				append(std::span<const Unit>{ std::ranges::data(units), std::ranges::size(units) });
			}
			else if constexpr (std::ranges::sized_range<R>)
			{
				if (state_->overflowed) [[unlikely]]
				{
					return;
				}

				const auto available = static_cast<std::size_t>(state_->end - state_->current);
				const auto requested = static_cast<std::size_t>(std::ranges::size(units));
				const auto to_write = (std::min)(available, requested);
				auto* out = std::to_address(state_->current);
				std::size_t written = 0;
				for (auto&& unit : units)
				{
					if (written == to_write) [[unlikely]]
					{
						break;
					}

					*out++ = static_cast<Unit>(unit);
					++written;
				}

				state_->current += static_cast<typename std::iterator_traits<Iterator>::difference_type>(written);
				if (written != requested) [[unlikely]]
				{
					state_->overflowed = true;
				}
			}
			else
			{
				for (auto&& unit : units)
				{
					push(static_cast<Unit>(unit));
				}
			}
		}

		[[nodiscard]]
		constexpr bool overflowed() const noexcept
		{
			return state_->overflowed;
		}

	private:
		bounded_output_state<Unit, Iterator, Sentinel>* state_ = nullptr;
	};

	template <typename Container, typename Unit>
	concept has_push_back =
		requires(Container& container, Unit unit)
	{
		container.push_back(unit);
	};

	template <typename Container, typename Unit>
	concept has_emplace_back =
		requires(Container& container, Unit unit)
	{
		container.emplace_back(unit);
	};

	template <typename Container, typename Unit>
	concept has_insert_at_end =
		requires(Container& container, Unit unit)
	{
		container.insert(container.end(), unit);
	};

	template <typename Container>
	concept has_reserve =
		requires(Container& container, std::size_t size)
	{
		container.reserve(size);
	};

	template <typename Container, typename Range>
	concept has_append_range =
		requires(Container& container, Range&& range)
	{
		container.append_range(std::forward<Range>(range));
	};

	template <typename Container, typename Range>
	concept has_insert_range_at_end =
		requires(Container& container, Range&& range)
	{
		container.insert_range(container.end(), std::forward<Range>(range));
	};

	template <typename Container, typename Iterator>
	concept has_insert_iterator_pair_at_end =
		requires(Container& container, Iterator first, Iterator last)
	{
		container.insert(container.end(), first, last);
	};

	template <typename Container, typename Unit>
	concept has_string_like_append =
		requires(Container& container, const Unit* data, std::size_t count)
	{
		container.append(data, count);
	};

	template <typename Container, typename Unit>
	concept has_resize_and_overwrite =
		std::same_as<typename Container::value_type, Unit>
		&& requires(Container& container, std::size_t size)
	{
		container.resize_and_overwrite(size,
			[](Unit* data, std::size_t) noexcept -> std::size_t
			{
				return static_cast<std::size_t>(data == nullptr ? 0 : 0);
			});
		};

	template <typename Container, typename Unit>
	concept sequence_like_append_container =
		requires
	{
		typename Container::value_type;
	}
		&& std::convertible_to<Unit, typename Container::value_type>
		&& (
			has_push_back<Container, typename Container::value_type>
			|| has_emplace_back<Container, typename Container::value_type>
			|| has_insert_at_end<Container, typename Container::value_type>);

	template <typename Unit, typename Container>
		requires sequence_like_append_container<Container, Unit>
	class container_append_writer
	{
	public:
		using unit_type = Unit;
		using container_type = Container;
		using value_type = typename Container::value_type;

		constexpr explicit container_append_writer(Container& container) noexcept
			: container_(&container)
		{
		}

		constexpr void reserve(std::size_t additional_units) const
		{
			if constexpr (has_reserve<Container>)
			{
				container_->reserve(container_->size() + additional_units);
			}
			else
			{
				(void)additional_units;
			}
		}

		constexpr void push(Unit unit) const
		{
			if constexpr (has_push_back<Container, value_type>)
			{
				container_->push_back(static_cast<value_type>(unit));
			}
			else if constexpr (has_emplace_back<Container, value_type>)
			{
				container_->emplace_back(static_cast<value_type>(unit));
			}
			else if constexpr (has_insert_at_end<Container, value_type>)
			{
				container_->insert(container_->end(), static_cast<value_type>(unit));
			}
			else
			{
				static_assert(has_push_back<Container, value_type>
					|| has_emplace_back<Container, value_type>
					|| has_insert_at_end<Container, value_type>,
					"container does not expose a supported append primitive");
			}
		}

		constexpr void append(std::span<const Unit> units) const
		{
			if (units.empty()) [[unlikely]]
			{
				return;
			}

			if constexpr (has_resize_and_overwrite<Container, Unit>)
			{
				const auto old_size = container_->size();
				container_->resize_and_overwrite(old_size + units.size(),
					[&](Unit* buffer, std::size_t) noexcept
					{
						std::char_traits<Unit>::copy(buffer + old_size, units.data(), units.size());
						return old_size + units.size();
					});
			}
			else if constexpr (has_append_range<Container, std::span<const Unit>>)
			{
				container_->append_range(units);
			}
			else if constexpr (has_string_like_append<Container, Unit>)
			{
				container_->append(units.data(), units.size());
			}
			else if constexpr (has_insert_range_at_end<Container, std::span<const Unit>>)
			{
				container_->insert_range(container_->end(), units);
			}
			else if constexpr (has_insert_iterator_pair_at_end<Container, typename std::span<const Unit>::iterator>)
			{
				container_->insert(container_->end(), units.begin(), units.end());
			}
			else
			{
				if constexpr (has_reserve<Container>)
				{
					container_->reserve(container_->size() + units.size());
				}

				for (const Unit unit : units)
				{
					push(unit);
				}
			}
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, Unit>
		constexpr void append(R&& units) const
		{
			if constexpr (
				std::ranges::contiguous_range<R>
				&& std::ranges::sized_range<R>
				&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, Unit>)
			{
				append(std::span<const Unit>{ std::ranges::data(units), std::ranges::size(units) });
			}
			else if constexpr (
				std::ranges::sized_range<R>
				&& has_resize_and_overwrite<Container, Unit>
				&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, Unit>)
			{
				const auto old_size = container_->size();
				const auto range_size = static_cast<std::size_t>(std::ranges::size(units));
				container_->resize_and_overwrite(old_size + range_size,
					[&](Unit* buffer, std::size_t) noexcept
					{
						auto* out = buffer + old_size;
						for (auto&& unit : units)
						{
							*out++ = static_cast<Unit>(unit);
						}

						return old_size + range_size;
					});
			}
			else if constexpr (has_append_range<Container, R>)
			{
				container_->append_range(std::forward<R>(units));
			}
			else if constexpr (has_insert_range_at_end<Container, R>)
			{
				container_->insert_range(container_->end(), std::forward<R>(units));
			}
			else if constexpr (has_insert_iterator_pair_at_end<Container, std::ranges::iterator_t<R>>)
			{
				container_->insert(container_->end(), std::ranges::begin(units), std::ranges::end(units));
			}
			else
			{
				if constexpr (std::ranges::sized_range<R> && has_reserve<Container>)
				{
					container_->reserve(container_->size() + static_cast<std::size_t>(std::ranges::size(units)));
				}

				for (auto&& unit : units)
				{
					push(static_cast<Unit>(unit));
				}
			}
		}

	private:
		Container* container_ = nullptr;
	};

	template <typename CodeUnitWriter>
	class utf8_scalar_writer
	{
	public:
		using unit_type = char32_t;

		constexpr explicit utf8_scalar_writer(CodeUnitWriter writer) noexcept
			: writer_(writer)
		{
		}

		constexpr void reserve(std::size_t additional_scalars) const
		{
			writer_.reserve(additional_scalars * details::encoding_constants::max_utf8_code_units);
		}

		constexpr void push(char32_t scalar) const
		{
			validate_codec_scalar(scalar);
			std::array<char8_t, details::encoding_constants::max_utf8_code_units> buffer{};
			const auto count = details::encode_unicode_scalar_utf8_unchecked(static_cast<std::uint32_t>(scalar), buffer.data());
			writer_.append(std::span<const char8_t>{ buffer.data(), count });
		}

		constexpr void append(std::span<const char32_t> scalars) const
		{
			if (!scalars.empty()) [[likely]]
			{
				reserve(scalars.size());
			}

			for (const char32_t scalar : scalars)
			{
				push(scalar);
			}
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, char32_t>
		constexpr void append(R&& scalars) const
		{
			if constexpr (std::ranges::sized_range<R>)
			{
				reserve(static_cast<std::size_t>(std::ranges::size(scalars)));
			}

			for (auto&& scalar : scalars)
			{
				push(static_cast<char32_t>(scalar));
			}
		}

	private:
		UTF8_RANGES_NO_UNIQUE_ADDRESS CodeUnitWriter writer_;
	};

	template <typename CodeUnitWriter>
	class utf16_scalar_writer
	{
	public:
		using unit_type = char32_t;

		constexpr explicit utf16_scalar_writer(CodeUnitWriter writer) noexcept
			: writer_(writer)
		{
		}

		constexpr void reserve(std::size_t additional_scalars) const
		{
			writer_.reserve(additional_scalars * details::encoding_constants::utf16_surrogate_code_unit_count);
		}

		constexpr void push(char32_t scalar) const
		{
			validate_codec_scalar(scalar);
			std::array<char16_t, details::encoding_constants::utf16_surrogate_code_unit_count> buffer{};
			const auto count = details::encode_unicode_scalar_utf16_unchecked(static_cast<std::uint32_t>(scalar), buffer.data());
			writer_.append(std::span<const char16_t>{ buffer.data(), count });
		}

		constexpr void append(std::span<const char32_t> scalars) const
		{
			if (!scalars.empty()) [[likely]]
			{
				reserve(scalars.size());
			}

			for (const char32_t scalar : scalars)
			{
				push(scalar);
			}
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, char32_t>
		constexpr void append(R&& scalars) const
		{
			if constexpr (std::ranges::sized_range<R>)
			{
				reserve(static_cast<std::size_t>(std::ranges::size(scalars)));
			}

			for (auto&& scalar : scalars)
			{
				push(static_cast<char32_t>(scalar));
			}
		}

	private:
		UTF8_RANGES_NO_UNIQUE_ADDRESS CodeUnitWriter writer_;
	};

	template <typename CodeUnitWriter>
	class utf32_scalar_writer
	{
	public:
		using unit_type = char32_t;

		constexpr explicit utf32_scalar_writer(CodeUnitWriter writer) noexcept
			: writer_(writer)
		{
		}

		constexpr void reserve(std::size_t additional_scalars) const
		{
			writer_.reserve(additional_scalars);
		}

		constexpr void push(char32_t scalar) const
		{
			validate_codec_scalar(scalar);
			writer_.push(static_cast<char32_t>(scalar));
		}

		constexpr void append(std::span<const char32_t> scalars) const
		{
			if (!scalars.empty()) [[likely]]
			{
				reserve(scalars.size());
			}

			for (const char32_t scalar : scalars)
			{
				push(scalar);
			}
		}

		template <std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, char32_t>
		constexpr void append(R&& scalars) const
		{
			if constexpr (std::ranges::sized_range<R>)
			{
				reserve(static_cast<std::size_t>(std::ranges::size(scalars)));
			}

			for (auto&& scalar : scalars)
			{
				push(static_cast<char32_t>(scalar));
			}
		}

	private:
		UTF8_RANGES_NO_UNIQUE_ADDRESS CodeUnitWriter writer_;
	};

	template <typename Decoder, typename ScalarWriter>
	constexpr auto decode_all_fallback(
		Decoder& decoder,
		std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
		ScalarWriter out)
		-> std::conditional_t<
			std::same_as<typename decoder_traits<Decoder>::decode_error, void>,
			void,
			std::expected<void, typename decoder_traits<Decoder>::decode_error>>
	{
		using traits_type = decoder_traits<Decoder>;
		using decode_error = typename traits_type::decode_error;

		std::size_t pos = 0;
		while (pos < input.size())
		{
			const auto remaining = input.substr(pos);
			if constexpr (std::same_as<decode_error, void>)
			{
				const auto consumed = traits_type::decode_one(decoder, remaining, out);
				validate_decode_progress(consumed, remaining.size());
				pos += consumed;
			}
			else
			{
				auto consumed = traits_type::decode_one(decoder, remaining, out);
				if (!consumed) [[unlikely]]
				{
					return std::expected<void, decode_error>{ std::unexpected(consumed.error()) };
				}

				validate_decode_progress(*consumed, remaining.size());
				pos += *consumed;
			}
		}

		if constexpr (!std::same_as<decode_error, void>)
		{
			return std::expected<void, decode_error>{};
		}
	}

	template <typename Encoder, typename Range, typename Writer>
	constexpr auto encode_all_fallback(Encoder& encoder, Range&& input, Writer out)
	{
		using traits_type = encoder_traits<Encoder>;
		using encode_error = typename traits_type::encode_error;

		if constexpr (std::same_as<encode_error, void>)
		{
			for (auto ch : input.chars())
			{
				traits_type::encode_one(encoder, static_cast<char32_t>(ch.as_scalar()), out);
			}
		}
		else
		{
			for (auto ch : input.chars())
			{
				auto result = traits_type::encode_one(encoder, static_cast<char32_t>(ch.as_scalar()), out);
				if (!result) [[unlikely]]
				{
					return result;
				}
			}

			return std::expected<void, encode_error>{};
		}
	}

	template <typename BaseString>
	constexpr void validate_utf_result(const BaseString& output)
	{
#if UTF8_RANGES_ENABLE_CODEC_CONTRACT_CHECKS
		if constexpr (std::same_as<typename BaseString::value_type, char8_t>)
		{
			if (!details::validate_utf8(std::u8string_view{ output }).has_value()) [[unlikely]]
			{
				on_codec_contract_violation<void>("decoder emitted malformed UTF-8");
			}
		}
		else if constexpr (std::same_as<typename BaseString::value_type, char16_t>)
		{
			if (!details::validate_utf16(std::u16string_view{ output }).has_value()) [[unlikely]]
			{
				on_codec_contract_violation<void>("decoder emitted malformed UTF-16");
			}
		}
		else if constexpr (std::same_as<typename BaseString::value_type, char32_t>)
		{
			if (!details::validate_utf32(std::u32string_view{ output }).has_value()) [[unlikely]]
			{
				on_codec_contract_violation<void>("decoder emitted malformed UTF-32");
			}
		}
#else
		(void)output;
#endif
	}

	template <typename Encoder, typename Writer>
	constexpr auto encode_whole_input(Encoder& encoder, utf8_string_view input, Writer out)
	{
		using encode_error = typename encoder_traits<Encoder>::encode_error;
		if constexpr (std::same_as<encode_error, void>)
		{
			encoder_traits<Encoder>::encode_from_utf8(encoder, input, out);
			encoder_traits<Encoder>::flush(encoder, out);
		}
		else
		{
			auto result = encoder_traits<Encoder>::encode_from_utf8(encoder, input, out);
			if (!result) [[unlikely]]
			{
				return result;
			}

			return encoder_traits<Encoder>::flush(encoder, out);
		}
	}

	template <typename Encoder, typename Writer>
	constexpr auto encode_whole_input(Encoder& encoder, utf16_string_view input, Writer out)
	{
		using encode_error = typename encoder_traits<Encoder>::encode_error;
		if constexpr (std::same_as<encode_error, void>)
		{
			encoder_traits<Encoder>::encode_from_utf16(encoder, input, out);
			encoder_traits<Encoder>::flush(encoder, out);
		}
		else
		{
			auto result = encoder_traits<Encoder>::encode_from_utf16(encoder, input, out);
			if (!result) [[unlikely]]
			{
				return result;
			}

			return encoder_traits<Encoder>::flush(encoder, out);
		}
	}

	template <typename Encoder, typename Writer>
	constexpr auto encode_whole_input(Encoder& encoder, utf32_string_view input, Writer out)
	{
		using encode_error = typename encoder_traits<Encoder>::encode_error;
		if constexpr (std::same_as<encode_error, void>)
		{
			encoder_traits<Encoder>::encode_from_utf32(encoder, input, out);
			encoder_traits<Encoder>::flush(encoder, out);
		}
		else
		{
			auto result = encoder_traits<Encoder>::encode_from_utf32(encoder, input, out);
			if (!result) [[unlikely]]
			{
				return result;
			}

			return encoder_traits<Encoder>::flush(encoder, out);
		}
	}

	template <typename Decoder, typename CodeUnitWriter, typename ScalarWriter>
	constexpr auto decode_whole_input_to_utf8(
		Decoder& decoder,
		std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
		CodeUnitWriter code_unit_out,
		ScalarWriter scalar_out)
	{
		using decode_error = typename decoder_traits<Decoder>::decode_error;
		if constexpr (std::same_as<decode_error, void>)
		{
			if (!input.empty()) [[likely]]
			{
				decoder_traits<Decoder>::decode_to_utf8(decoder, input, code_unit_out);
			}
			decoder_traits<Decoder>::flush(decoder, scalar_out);
		}
		else
		{
			if (!input.empty()) [[likely]]
			{
				auto result = decoder_traits<Decoder>::decode_to_utf8(decoder, input, code_unit_out);
				if (!result) [[unlikely]]
				{
					return result;
				}
			}

			return decoder_traits<Decoder>::flush(decoder, scalar_out);
		}
	}

	template <typename Decoder, typename CodeUnitWriter, typename ScalarWriter>
	constexpr auto decode_whole_input_to_utf16(
		Decoder& decoder,
		std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
		CodeUnitWriter code_unit_out,
		ScalarWriter scalar_out)
	{
		using decode_error = typename decoder_traits<Decoder>::decode_error;
		if constexpr (std::same_as<decode_error, void>)
		{
			if (!input.empty()) [[likely]]
			{
				decoder_traits<Decoder>::decode_to_utf16(decoder, input, code_unit_out);
			}
			decoder_traits<Decoder>::flush(decoder, scalar_out);
		}
		else
		{
			if (!input.empty()) [[likely]]
			{
				auto result = decoder_traits<Decoder>::decode_to_utf16(decoder, input, code_unit_out);
				if (!result) [[unlikely]]
				{
					return result;
				}
			}

			return decoder_traits<Decoder>::flush(decoder, scalar_out);
		}
	}

	template <typename Decoder, typename CodeUnitWriter, typename ScalarWriter>
	constexpr auto decode_whole_input_to_utf32(
		Decoder& decoder,
		std::basic_string_view<typename decoder_traits<Decoder>::code_unit_type> input,
		CodeUnitWriter code_unit_out,
		ScalarWriter scalar_out)
	{
		using decode_error = typename decoder_traits<Decoder>::decode_error;
		if constexpr (std::same_as<decode_error, void>)
		{
			if (!input.empty()) [[likely]]
			{
				decoder_traits<Decoder>::decode_to_utf32(decoder, input, code_unit_out);
			}
			decoder_traits<Decoder>::flush(decoder, scalar_out);
		}
		else
		{
			if (!input.empty()) [[likely]]
			{
				auto result = decoder_traits<Decoder>::decode_to_utf32(decoder, input, code_unit_out);
				if (!result) [[unlikely]]
				{
					return result;
				}
			}

			return decoder_traits<Decoder>::flush(decoder, scalar_out);
		}
	}

} // namespace details

template <typename Encoder>
struct encoder_traits
{
	using code_unit_type = typename Encoder::code_unit_type;
	using encode_error = details::detected_t<Encoder, details::codec_encode_error_t>;

	static constexpr bool has_explicit_allow_implicit_construction =
		details::has_explicit_allow_implicit_construction_v<Encoder>;
	static constexpr bool allow_implicit_construction_requested =
		details::allow_implicit_construction_requested_v<Encoder>;

	template <typename Writer>
	static constexpr decltype(auto) encode_one(Encoder& encoder, char32_t scalar, Writer out)
	{
		return encoder.encode_one(scalar, out);
	}

	template <typename Writer>
	static constexpr auto flush(Encoder& encoder, Writer out)
	{
		if constexpr (requires { encoder.flush(out); })
		{
			if constexpr (std::same_as<encode_error, void>)
			{
				encoder.flush(out);
			}
			else
			{
				return encoder.flush(out);
			}
		}
		else if constexpr (!std::same_as<encode_error, void>)
		{
			return std::expected<void, encode_error>{};
		}
	}

	template <typename Writer>
	static constexpr auto encode_from_utf8(Encoder& encoder, utf8_string_view input, Writer out)
	{
		if constexpr (requires { encoder.encode_from_utf8(input, out); })
		{
			if constexpr (std::same_as<encode_error, void>)
			{
				encoder.encode_from_utf8(input, out);
			}
			else
			{
				return encoder.encode_from_utf8(input, out);
			}
		}
		else
		{
			return details::encode_all_fallback(encoder, input, out);
		}
	}

	template <typename Writer>
	static constexpr auto encode_from_utf16(Encoder& encoder, utf16_string_view input, Writer out)
	{
		if constexpr (requires { encoder.encode_from_utf16(input, out); })
		{
			if constexpr (std::same_as<encode_error, void>)
			{
				encoder.encode_from_utf16(input, out);
			}
			else
			{
				return encoder.encode_from_utf16(input, out);
			}
		}
		else
		{
			return details::encode_all_fallback(encoder, input, out);
		}
	}

	template <typename Writer>
	static constexpr auto encode_from_utf32(Encoder& encoder, utf32_string_view input, Writer out)
	{
		if constexpr (requires { encoder.encode_from_utf32(input, out); })
		{
			if constexpr (std::same_as<encode_error, void>)
			{
				encoder.encode_from_utf32(input, out);
			}
			else
			{
				return encoder.encode_from_utf32(input, out);
			}
		}
		else
		{
			return details::encode_all_fallback(encoder, input, out);
		}
	}
};

template <typename Decoder>
struct decoder_traits
{
	using code_unit_type = typename Decoder::code_unit_type;
	using decode_error = details::detected_t<Decoder, details::codec_decode_error_t>;

	static constexpr bool has_explicit_allow_implicit_construction =
		details::has_explicit_allow_implicit_construction_v<Decoder>;
	static constexpr bool allow_implicit_construction_requested =
		details::allow_implicit_construction_requested_v<Decoder>;

	template <typename Writer>
	static constexpr decltype(auto) decode_one(
		Decoder& decoder,
		std::basic_string_view<code_unit_type> input,
		Writer out)
	{
		return decoder.decode_one(input, out);
	}

	template <typename Writer>
	static constexpr auto flush(Decoder& decoder, Writer out)
	{
		if constexpr (requires { decoder.flush(out); })
		{
			if constexpr (std::same_as<decode_error, void>)
			{
				decoder.flush(out);
			}
			else
			{
				return decoder.flush(out);
			}
		}
		else if constexpr (!std::same_as<decode_error, void>)
		{
			return std::expected<void, decode_error>{};
		}
	}

	template <typename Writer>
	static constexpr auto decode_to_utf8(
		Decoder& decoder,
		std::basic_string_view<code_unit_type> input,
		Writer out)
	{
		if constexpr (requires { decoder.decode_to_utf8(input, out); })
		{
			if constexpr (std::same_as<decode_error, void>)
			{
				decoder.decode_to_utf8(input, out);
			}
			else
			{
				return decoder.decode_to_utf8(input, out);
			}
		}
		else
		{
			return details::decode_all_fallback(decoder, input, details::utf8_scalar_writer<Writer>{ out });
		}
	}

	template <typename Writer>
	static constexpr auto decode_to_utf16(
		Decoder& decoder,
		std::basic_string_view<code_unit_type> input,
		Writer out)
	{
		if constexpr (requires { decoder.decode_to_utf16(input, out); })
		{
			if constexpr (std::same_as<decode_error, void>)
			{
				decoder.decode_to_utf16(input, out);
			}
			else
			{
				return decoder.decode_to_utf16(input, out);
			}
		}
		else
		{
			return details::decode_all_fallback(decoder, input, details::utf16_scalar_writer<Writer>{ out });
		}
	}

	template <typename Writer>
	static constexpr auto decode_to_utf32(
		Decoder& decoder,
		std::basic_string_view<code_unit_type> input,
		Writer out)
	{
		if constexpr (requires { decoder.decode_to_utf32(input, out); })
		{
			if constexpr (std::same_as<decode_error, void>)
			{
				decoder.decode_to_utf32(input, out);
			}
			else
			{
				return decoder.decode_to_utf32(input, out);
			}
		}
		else
		{
			return details::decode_all_fallback(decoder, input, details::utf32_scalar_writer<Writer>{ out });
		}
	}
};

template <typename T>
concept encoder =
	requires(T& value)
{
	typename encoder_traits<T>::code_unit_type;
	encoder_traits<T>::encode_one(
		value,
		char32_t{},
		details::probe_code_unit_writer<typename encoder_traits<T>::code_unit_type>{});
};

template <typename T>
concept decoder =
	requires(T& value)
{
	typename decoder_traits<T>::code_unit_type;
	decoder_traits<T>::decode_one(
		value,
		std::basic_string_view<typename decoder_traits<T>::code_unit_type>{},
		details::probe_scalar_writer{});
};

namespace encodings
{

struct ascii_strict
{
	using code_unit_type = char8_t;

	enum class encode_error
	{
		unrepresentable_scalar
	};

	enum class decode_error
	{
		invalid_input
	};

	static constexpr bool allow_implicit_construction = true;

	template <typename Writer>
	constexpr auto encode_one(char32_t scalar, Writer out) -> std::expected<void, encode_error>
	{
		if (scalar > 0x7F)
		{
			return std::unexpected(encode_error::unrepresentable_scalar);
		}

		out.push(static_cast<code_unit_type>(scalar));
		return {};
	}

	template <typename Writer>
	constexpr auto decode_one(std::basic_string_view<code_unit_type> input, Writer out)
		-> std::expected<std::size_t, decode_error>
	{
		const auto byte = input.front();
		if (byte > 0x7F)
		{
			return std::unexpected(decode_error::invalid_input);
		}

		out.push(static_cast<char32_t>(byte));
		return 1;
	}

	template <typename Writer>
	constexpr auto encode_from_utf8(utf8_string_view input, Writer out) -> std::expected<void, encode_error>
	{
		const auto bytes = input.base();
		for (char8_t byte : bytes)
		{
			if (static_cast<std::uint8_t>(byte) > 0x7F)
			{
				return std::unexpected(encode_error::unrepresentable_scalar);
			}
		}

		std::span<const char8_t> view{ bytes.data(), bytes.size() };
		out.append(view | std::views::transform([](char8_t byte) { return static_cast<code_unit_type>(byte); }));
		return {};
	}

	template <typename Writer>
	constexpr auto decode_to_utf8(std::basic_string_view<code_unit_type> input, Writer out)
		-> std::expected<void, decode_error>
	{
		for (code_unit_type byte : input)
		{
			if (byte > 0x7F)
			{
				return std::unexpected(decode_error::invalid_input);
			}
		}

		out.append(input | std::views::transform([](code_unit_type byte) { return static_cast<char8_t>(byte); }));
		return {};
	}
};

struct ascii_lossy
{
	using code_unit_type = char8_t;

	std::size_t replacement_count = 0;

	template <typename Writer>
	constexpr void encode_one(char32_t scalar, Writer out)
	{
		if (scalar <= 0x7F)
		{
			out.push(static_cast<code_unit_type>(scalar));
			return;
		}

		out.push(static_cast<code_unit_type>('?'));
		++replacement_count;
	}

	template <typename Writer>
	constexpr std::size_t decode_one(std::basic_string_view<code_unit_type> input, Writer out)
	{
		const auto byte = input.front();
		if (byte <= 0x7F)
		{
			out.push(static_cast<char32_t>(byte));
		}
		else
		{
			out.push(static_cast<char32_t>(details::encoding_constants::replacement_character_scalar));
			++replacement_count;
		}

		return 1;
	}
};

} // namespace encodings

} // namespace unicode_ranges

#endif // UTF8_RANGES_ENCODING_HPP
