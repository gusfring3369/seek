#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "seek/trie.hpp"
#include "seek/types.hpp"

namespace seek {

struct FuzzyMatch {
    std::string word;
    std::size_t distance = 0;  // Levenshtein distance to the query
    Weight weight = 0;

    friend bool operator==(const FuzzyMatch&, const FuzzyMatch&) = default;
};

// Work counters, used to measure how much of the trie the pruning skips.
struct FuzzyStats {
    std::size_t nodesVisited = 0;
    std::size_t dpCellsComputed = 0;
};

// Every word within `maxDistance` Levenshtein edits (insert / delete / substitute) of `query`,
// ordered by distance, then weight (descending), then alphabetically.
//
// Instead of running an O(|w| * |q|) DP per dictionary word, a DFS over the trie computes one
// DP row per node; words that share a prefix share those rows. A subtree is skipped as soon as
// the smallest value in the current row exceeds `maxDistance`, because row minima never
// decrease with depth, so no word below can be close enough.
[[nodiscard]] std::vector<FuzzyMatch> fuzzySearch(const Trie& trie, std::string_view query,
                                                  std::size_t maxDistance,
                                                  FuzzyStats* stats = nullptr);

}  // namespace seek
