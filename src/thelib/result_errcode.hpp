#pragma once
#include <type_traits>

namespace lib {
/// A result errcode is an enum class which contains a definition for Okay and
/// ResultReleased, and has a one-byte underlying type.
template <typename T>
concept result_errcode_c = std::is_enum_v<T> && sizeof(T) == 1 &&
                           (requires(T code) {
                               T::Okay;
                               T::ResultReleased;
                           }) &&
                           std::underlying_type_t<T>(T::Okay) == 0 &&
                           (std::underlying_type_t<T>(T::ResultReleased) !=
                            std::underlying_type_t<T>(T::Okay));
} // namespace lib
