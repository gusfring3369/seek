#pragma once

#include <map>
#include <memory>

#include "seek/common/types.hpp"

namespace seek {

// One node of a Trie. A node stands for the prefix spelled by the path from the root to it.
//
// Nodes are read-only to everyone except the Trie that owns them, so algorithms that walk the
// trie (autocomplete, spell checking) cannot break its invariants.
class TrieNode {
public:
    // Children keyed by the next character. std::map keeps them sorted, so every traversal
    // visits words in alphabetical order and results are deterministic.
    using Children = std::map<char, std::unique_ptr<TrieNode>>;

    [[nodiscard]] const Children& children() const noexcept { return children_; }

    // The child reached by character `c`, or nullptr if there is none.
    [[nodiscard]] const TrieNode* child(char c) const {
        const auto it = children_.find(c);
        return it == children_.end() ? nullptr : it->second.get();
    }

    // Total weight of the word that ends at this node, or 0 if no word ends here.
    [[nodiscard]] Weight weight() const noexcept { return weight_; }
    [[nodiscard]] bool isEndOfWord() const noexcept { return weight_ > 0; }

private:
    friend class Trie;  // the only class allowed to modify nodes

    Children children_;
    Weight weight_ = 0;
};

}  // namespace seek
