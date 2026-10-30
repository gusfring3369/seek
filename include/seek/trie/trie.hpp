#pragma once

#include <cstddef>
#include <string_view>

#include "seek/common/types.hpp"
#include "seek/trie/trie_node.hpp"

namespace seek {

// A prefix tree that maps words to weights.
//
//     root
//      |-- d -- a -- t*                  * = a word ends here
//      `-- t -- e -- a*                  words: "dat", "tea", "ten"
//                `-- n*
//
// Words that share a prefix share the nodes for it, so finding every word that starts with a
// prefix is a walk down to the prefix's node followed by a scan of its subtree.
//
// Nodes do not store the word they spell. Traversals rebuild it from the path, which keeps memory
// proportional to the number of distinct prefixes rather than the total length of all words.
class Trie {
public:
    Trie() = default;

    // Adds `weight` to `word` (inserting it if new) and returns the word's new total. O(length).
    // Throws std::invalid_argument for an empty word or a zero weight.
    Weight insert(std::string_view word, Weight weight = 1);

    // Total weight of `word`, or 0 if it was never inserted. O(length).
    [[nodiscard]] Weight weightOf(std::string_view word) const;
    [[nodiscard]] bool contains(std::string_view word) const { return weightOf(word) > 0; }

    // The node for `prefix`, or nullptr if no inserted word starts with it. O(length).
    [[nodiscard]] const TrieNode* find(std::string_view prefix) const;

    [[nodiscard]] const TrieNode& root() const noexcept { return root_; }
    [[nodiscard]] std::size_t wordCount() const noexcept { return wordCount_; }
    [[nodiscard]] std::size_t nodeCount() const noexcept { return nodeCount_; }

private:
    TrieNode root_;
    std::size_t wordCount_ = 0;
    std::size_t nodeCount_ = 1;  // the root
};

}  // namespace seek
