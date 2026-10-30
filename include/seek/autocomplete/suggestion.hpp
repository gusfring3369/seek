#pragma once

#include <string>
#include <string_view>

#include "seek/common/types.hpp"

namespace seek {

// One autocomplete result.
struct Suggestion {
    std::string word;
    Weight weight = 0;

    // The ranking used everywhere in autocomplete: higher weight first, ties alphabetical.
    // This overload takes the fields separately so a candidate can be compared before its word
    // is copied into a Suggestion.
    [[nodiscard]] static bool ranksAbove(Weight weightA, std::string_view wordA, Weight weightB,
                                         std::string_view wordB) noexcept {
        if (weightA != weightB) return weightA > weightB;
        return wordA < wordB;
    }

    [[nodiscard]] bool ranksAbove(const Suggestion& other) const noexcept {
        return ranksAbove(weight, word, other.weight, other.word);
    }

    friend bool operator==(const Suggestion&, const Suggestion&) = default;
};

}  // namespace seek
