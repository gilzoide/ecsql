#pragma once

#include <concepts>
#include <optional>

template<typename T>
concept is_optional = std::same_as<T, std::optional<typename T::value_type>>;
