#pragma once

#include <span>

template<typename T>
concept is_span = std::same_as<T, std::span<typename T::value_type>>;
