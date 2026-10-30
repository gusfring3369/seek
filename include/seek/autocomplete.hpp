#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "seek/trie.hpp"
#include "seek/types.hpp"

namespace seek {

struct Suggestion {
    std::string word;
    Weight weight = 0;

    friend bool operator==(const Suggestion&, const Suggestion&) = default;
};

// Suggestion order: higher weight first, ties broken alphabetically. Takes the fields
// separately so candidates can be compared before their word is copied into a Suggestion.
[[nodiscard]] inline bool ranksAbove(Weight weightA, std::string_view wordA, Weight weightB,
                                     std::string_view wordB) noexcept {
    if (weightA != weightB) return weightA > weightB;
    return wordA < wordB;
}

[[nodiscard]] inline bool ranksAbove(const Suggestion& a, const Suggestion& b) noexcept {
    return ranksAbove(a.weight, a.word, b.weight, b.word);
}

// The `k` highest-weight words that start with `prefix`, best first (ties: alphabetical).
//
// Walks to the prefix node in O(L), then scans its M-node subtree while keeping a bounded
// min-heap of the best k candidates: O(L + M log k) time, O(k) extra space.
[[nodiscard]] std::vector<Suggestion> topKCompletions(const Trie& trie, std::string_view prefix,
                                                      std::size_t k);

}  // namespace seek
