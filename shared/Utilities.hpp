#pragma once

#include "./_config.h"
#include "songloader/shared/API.hpp"

namespace MultiplayerCore {
    struct MPCORE_EXPORT Utilities {
        static inline std::string HashForLevelId(const std::string& levelId) {
            return levelId.starts_with(RuntimeSongLoader::API::GetCustomLevelsPrefix()) ? levelId.substr(RuntimeSongLoader::API::GetCustomLevelsPrefix().length()) : "";
        }
    };
}
