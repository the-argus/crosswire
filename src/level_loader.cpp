#include "level_loader.hpp"
#include "crosswire_editor/serialize.h"
#include "natural_log/natural_log.hpp"

namespace cw::loader {
void load_level(const char *levelname) noexcept
{
    Level level;
    std::array<char, 1024> buf;
    fmt::format_to(buf.begin(), "levels/{}.cwl", levelname);
    auto res = cw::deserialize(buf.data(), &level);

    if (res != decltype(res)::Okay) {
        LN_ERROR_FMT("Failed loading level {} due to errcode {}", levelname,
                     fmt::underlying(res));
        return;
    }
}
} // namespace cw::loader
