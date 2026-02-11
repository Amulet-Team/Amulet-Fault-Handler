#pragma once

#include <filesystem>

namespace Amulet {
namespace faulthandler {
    void install(std::filesystem::path path, bool full_dump);
}
}
