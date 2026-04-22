#ifndef UTF8_RANGES_UNICODE_TABLES_HPP
#define UTF8_RANGES_UNICODE_TABLES_HPP

// Compatibility wrapper: the generated constexpr Unicode data currently lives in
// unicode_tables_constexpr.hpp. Keeping this include path stable lets the table
// generator split runtime-only accelerators out later without breaking callers.
#include "unicode_tables_constexpr.hpp"

#endif
