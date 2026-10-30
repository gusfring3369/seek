#include "seek/trie.hpp"

#include <stdexcept>

namespace seek {

Weight Trie::insert(std::string_view word, Weight weight) {
    if (word.empty()) throw std::invalid_argument("Trie::insert: empty word");
    if (weight == 0) throw std::invalid_argument("Trie::insert: weight must be positive");

    Node* node = &root_;
    for (char c : word) {
        auto& child = node->children[c];
        if (!child) {
            child = std::make_unique<Node>();
            ++nodeCount_;
        }
        node = child.get();
    }
    if (!node->isWord()) ++wordCount_;
    node->weight += weight;
    return node->weight;
}

Weight Trie::weight(std::string_view word) const {
    const Node* node = find(word);
    return node != nullptr ? node->weight : 0;
}

const Trie::Node* Trie::find(std::string_view prefix) const {
    const Node* node = &root_;
    for (char c : prefix) {
        auto it = node->children.find(c);
        if (it == node->children.end()) return nullptr;
        node = it->second.get();
    }
    return node;
}

}  // namespace seek
