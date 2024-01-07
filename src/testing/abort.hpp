#pragma once
// Defines the ABORT() macro which throws an exception in debug mode or just
// calls std::abort in release mode.

#ifdef TESTING
#include <exception>
namespace reserve {
class _abort_exception : std::exception
{
  public:
    // NOLINTNEXTLINE
    char *what() { return "Program failure."; }
};
} // namespace reserve
#define ABORT() throw reserve::_abort_exception()
#else
#define ABORT() std::abort()
#endif
