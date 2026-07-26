#pragma once

#include <stdx/result.hh>

namespace ray {

enum class error {};

template <typename T> using result = stdx::result<T, error>;

} // namespace ray
