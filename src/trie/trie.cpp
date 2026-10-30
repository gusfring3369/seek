#include "seek/trie/trie.hpp"

#include <stdexcept>

using namespace std;

namespace seek {

Weight Trie::insert(string_view word, Weight weight) {
    if (word.empty()) throw invalid_argument("Trie::insert: empty word");
    if (weight == 0) throw invalid_argument("Trie::insert: weight must be positive");

    TrieNode* node = &root_;
    for (char c : word) {
        unique_ptr<TrieNode>& child = node->children_[c];
        if (!child) {
            child = make_unique<TrieNode>();
            ++nodeCount_;
        }
        node = child.get();
    }
    if (!node->isEndOfWord()) ++wordCount_;
    node->weight_ += weight;
    return node->weight_;
}

Weight Trie::weightOf(string_view word) const {
    const TrieNode* node = find(word);
    return node != nullptr ? node->weight() : 0;
}

const TrieNode* Trie::find(string_view prefix) const {
    const TrieNode* node = &root_;
    for (char c : prefix) {
        node = node->child(c);
        if (node == nullptr) return nullptr;
    }
    return node;
}

}  // namespace seek
