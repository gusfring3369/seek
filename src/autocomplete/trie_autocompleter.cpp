#include "seek/autocomplete/trie_autocompleter.hpp"

using namespace std;

namespace seek {

vector<Suggestion> TrieAutocompleter::complete(string_view prefix, size_t k) const {
    const TrieNode* start = trie_.find(prefix);
    if (start == nullptr || k == 0) return {};

    TopKHeap heap(k);
    string path(prefix);
    collect(*start, path, heap);
    return heap.takeBestFirst();
}

void TrieAutocompleter::collect(const TrieNode& node, string& path, TopKHeap& heap) {
    if (node.isEndOfWord()) heap.offer(path, node.weight());
    for (const auto& [c, child] : node.children()) {
        path.push_back(c);
        collect(*child, path, heap);
        path.pop_back();
    }
}

}  // namespace seek
