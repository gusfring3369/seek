#pragma once

#include <cstddef>
#include <functional>
#include <string_view>

namespace seek {

// Hash for std::unordered_map<std::string, ...> that also accepts std::string_view keys.
// Paired with std::equal_to<>, it lets map.find(someStringView) work without first copying the
// key into a temporary std::string.
struct StringHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view text) const noexcept {
        return std::hash<std::string_view>{}(text);
    }
};

}  // namespace seek
