#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "seek/common/types.hpp"
#include "seek/trie/trie.hpp"

namespace seek {

// One "did you mean?" result.
struct Correction {
    std::string word;
    std::size_t distance = 0;  // edits away from the misspelled word
    Weight weight = 0;

    friend bool operator==(const Correction&, const Correction&) = default;
};

// How much work one suggest() call did. Used by tests and benchmarks to measure the pruning.
struct SpellCheckStats {
    std::size_t nodesVisited = 0;
    std::size_t dpCellsComputed = 0;
};

// Finds dictionary words within a few typos of a given word ("did you mean?").
//
// The naive way runs EditDistance::between(word, w) for every dictionary word w. This class
// walks the trie depth-first instead and computes one EditDistance row per node it enters:
//   * Words with a common prefix share the rows for that prefix, so they are computed once.
//   * If every value in a node's row is already above maxDistance, no word below that node can
//     match (values never shrink further down), so the whole subtree is skipped.
// On a 370k-word dictionary this computes well under 1% of the naive DP cells (see README).
class SpellChecker {
public:
    explicit SpellChecker(const Trie& dictionary) : dictionary_(dictionary) {}

    // Every dictionary word within `maxDistance` edits of `word`: closest first, then most
    // popular, then alphabetical. Pass `stats` to find out how much work was done.
    [[nodiscard]] std::vector<Correction> suggest(std::string_view word, std::size_t maxDistance,
                                                  SpellCheckStats* stats = nullptr) const;

private:
    class Search;  // the state of one suggest() call; defined in spell_checker.cpp

    const Trie& dictionary_;
};

}  // namespace seek
