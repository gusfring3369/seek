#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string_view>

#include "seek/types.hpp"

namespace seek {

// Prefix tree mapping words to accumulated weights.
//
// Nodes do not store the word they spell; traversals rebuild it from the path, which keeps
// memory proportional to the number of distinct prefixes rather than total word length.
// The trie only owns the data: ranking (autocomplete) and approximate matching (fuzzy) are
// separate algorithms that walk it through the read-only Node interface.
class Trie {
public:
    struct Node {
        // std::map keeps children ordered, so every traversal is deterministic.
        std::map<char, std::unique_ptr<Node>> children;
        Weight weight = 0;  // > 0 exactly when a word ends at this node

        [[nodiscard]] bool isWord() const noexcept { return weight > 0; }
    };

    Trie() = default;
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;
    Trie(Trie&&) noexcept = default;
    Trie& operator=(Trie&&) noexcept = default;
    ~Trie() = default;

    // Adds `weight` to `word`, inserting it if absent, and returns the word's new total. O(L).
    // Throws std::invalid_argument for an empty word or a zero weight.
    Weight insert(std::string_view word, Weight weight = 1);

    // Accumulated weight of `word`, or 0 if it was never inserted. O(L).
    [[nodiscard]] Weight weight(std::string_view word) const;
    [[nodiscard]] bool contains(std::string_view word) const { return weight(word) > 0; }

    // Node reached by spelling `prefix` from the root, or nullptr if no word has that prefix.
    [[nodiscard]] const Node* find(std::string_view prefix) const;

    [[nodiscard]] const Node& root() const noexcept { return root_; }
    [[nodiscard]] std::size_t wordCount() const noexcept { return wordCount_; }
    [[nodiscard]] std::size_t nodeCount() const noexcept { return nodeCount_; }

private:
    Node root_;
    std::size_t wordCount_ = 0;
    std::size_t nodeCount_ = 1;  // the root
};

}  // namespace seek
