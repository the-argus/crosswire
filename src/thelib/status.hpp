#pragma once
#include "thelib/result_errcode.hpp"

namespace lib {
/// Wrapper around a errcode to give it a similar interface to a result_t.
template <result_errcode_c errcode_e> struct status_t
{
  private:
    errcode_e m_status;

  public:
    [[nodiscard]] inline constexpr bool okay() const TESTING_NOEXCEPT
    {
        return m_status == errcode_e::Okay;
    }
    /// Effectively unwraps the status into its underlying type. Should be a
    /// no-op when compiled in release mode
    [[nodiscard]] inline constexpr errcode_e status() const TESTING_NOEXCEPT
    {
        return m_status;
    }

    /// A status_t can be implicitly constructed from a errcode_e. Should be a
    /// no-op in release mode.
    inline constexpr status_t(errcode_e failure) TESTING_NOEXCEPT
    {
        m_status = failure;
    }
};
} // namespace lib
