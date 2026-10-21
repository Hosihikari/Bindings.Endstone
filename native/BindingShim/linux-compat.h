#pragma once

// Endstone 0.11.14 uses std::floorf in level/location.h. The MSVC STL exports it,
// while libstdc++ only guarantees the std::floor overloads. This compatibility
// header is force-included for the Linux shim translation unit only.
#include <cmath>

namespace std {
using ::floorf;
}
