#ifndef UTF8_RANGES_INTERNAL_UTF32_PARALLEL_RUNTIME_HPP
#define UTF8_RANGES_INTERNAL_UTF32_PARALLEL_RUNTIME_HPP

#ifndef UTF8_RANGES_ENABLE_TEST_HOOKS
#define UTF8_RANGES_ENABLE_TEST_HOOKS 0
#endif

#ifndef UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL
#define UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL 0
#endif

namespace unicode_ranges::details
{
#if UINTPTR_MAX > 0xFFFFFFFFu
	inline constexpr std::size_t runtime_parallel_min_total_bytes = 1u << 20;
	inline constexpr std::size_t runtime_parallel_min_bytes_per_worker = 1u << 18;
	// The UTF-32 parallel paths should never try to scale to an unbounded worker
	// count. A modest cap keeps scheduling overhead in check and makes the worker
	// array size provably bounded for aggressive whole-program optimizers.
	inline constexpr std::size_t runtime_parallel_max_worker_count = 64;
#else
	// 32-bit runners have noticeably higher thread/setup pressure relative to the
	// UTF-32 workloads we parallelize here, so they use a stricter activation policy.
	inline constexpr std::size_t runtime_parallel_min_total_bytes = 2u << 20;
	inline constexpr std::size_t runtime_parallel_min_bytes_per_worker = 1u << 20;
	inline constexpr std::size_t runtime_parallel_max_worker_count = 2;
#endif

#if UTF8_RANGES_ENABLE_TEST_HOOKS
	inline constexpr bool test_force_utf32_parallel = UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL != 0;
#else
	inline constexpr bool test_force_utf32_parallel = false;
#endif

	struct utf32_parallel_plan
	{
		std::size_t worker_count = 1;
		std::size_t chunk_size = 0;
	};

	[[nodiscard]]
	inline constexpr utf32_parallel_plan make_utf32_parallel_plan(
		std::size_t code_point_count,
		std::size_t available_workers) noexcept
	{
		if (code_point_count == 0)
		{
			return { 1, code_point_count };
		}

		if (test_force_utf32_parallel && available_workers >= 2)
		{
			const auto forced_workers = std::max<std::size_t>(
				2,
				std::min(available_workers, runtime_parallel_max_worker_count));
			return {
				forced_workers,
				(code_point_count + forced_workers - 1) / forced_workers
			};
		}

		const auto total_bytes = code_point_count * sizeof(char32_t);
		if (total_bytes < runtime_parallel_min_total_bytes)
		{
			return { 1, code_point_count };
		}

		// UTF-32 is fixed-width, so chunking by code-point index is always boundary-safe.
		std::size_t worker_count = available_workers;
		if (worker_count < 2)
		{
			return { 1, code_point_count };
		}

		worker_count = std::min(worker_count, runtime_parallel_max_worker_count);
		worker_count = std::min(worker_count, total_bytes / runtime_parallel_min_bytes_per_worker);
		worker_count = std::min(worker_count, code_point_count);
		if (worker_count < 2)
		{
			return { 1, code_point_count };
		}

		return {
			worker_count,
			(code_point_count + worker_count - 1) / worker_count
		};
	}

	[[nodiscard]]
	utf32_parallel_plan make_utf32_parallel_plan(std::size_t code_point_count) noexcept;

	[[nodiscard]]
	inline constexpr std::u32string_view utf32_parallel_chunk(
		std::u32string_view code_points,
		utf32_parallel_plan plan,
		std::size_t chunk_index) noexcept
	{
		// Each worker gets a disjoint contiguous subspan so it can measure and write
		// independently without any boundary repair.
		const auto start = chunk_index * plan.chunk_size;
		if (start >= code_points.size())
		{
			return {};
		}

		return code_points.substr(start, std::min(plan.chunk_size, code_points.size() - start));
	}

	using utf32_parallel_job_fn = void(*)(void*, std::size_t) noexcept;

	void run_parallel_jobs_runtime(
		std::size_t worker_count,
		void* context,
		utf32_parallel_job_fn job);

	template <typename Fn>
	inline void run_parallel_jobs(std::size_t worker_count, Fn&& fn)
	{
		using fn_type = std::remove_reference_t<Fn>;
		static_assert(std::is_nothrow_invocable_v<fn_type&, std::size_t>);

		struct context
		{
			fn_type* fn = nullptr;
		};

		context job_context{ std::addressof(fn) };
		run_parallel_jobs_runtime(
			worker_count,
			&job_context,
			[](void* raw_context, std::size_t worker_index) noexcept
			{
				auto& typed_context = *static_cast<context*>(raw_context);
				(*typed_context.fn)(worker_index);
			});
	}

	template <typename Result, typename MeasureFn>
	inline void fill_parallel_utf32_chunk_results(
		std::u32string_view code_points,
		utf32_parallel_plan plan,
		Result* chunk_results,
		MeasureFn&& measure_fn)
	{
		// First pass: let each worker compute metadata for its own chunk. The caller
		// later turns these per-chunk measurements into a global prefix-sum.
		run_parallel_jobs(plan.worker_count,
			[&](std::size_t chunk_index) noexcept
			{
				chunk_results[chunk_index] = measure_fn(utf32_parallel_chunk(code_points, plan, chunk_index));
			});
	}

	template <typename CodeUnit, typename WriteFn>
	inline void write_parallel_utf32_chunks(
		std::u32string_view code_points,
		utf32_parallel_plan plan,
		const std::size_t* chunk_offsets,
		CodeUnit* buffer,
		WriteFn&& write_fn)
	{
		// Second pass: each worker writes into the exact output slice reserved for its
		// chunk, so no synchronization is needed during emission.
		run_parallel_jobs(plan.worker_count,
			[&](std::size_t chunk_index) noexcept
			{
				const auto chunk = utf32_parallel_chunk(code_points, plan, chunk_index);
				if (!chunk.empty())
				{
					write_fn(chunk, buffer + chunk_offsets[chunk_index]);
				}
			});
	}
}

#endif
