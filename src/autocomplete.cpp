#include "seek/autocomplete.hpp"

#include <algorithm>
#include <queue>

namespace seek {
namespace {

struct RanksAbove {
    bool operator()(const Suggestion& a, const Suggestion& b) const { return ranksAbove(a, b); }
};

// With "ranks above" as the heap's less-than, top() is the weakest of the current top k,
// which is exactly the element to evict when a better candidate shows up.
using BoundedMinHeap = std::priority_queue<Suggestion, std::vector<Suggestion>, RanksAbove>;

class TopKCollector {
public:
    explicit TopKCollector(std::size_t k) : k_(k) {}

    void collect(const Trie::Node& node, std::string& path) {
        if (node.isWord()) offer(node.weight, path);
        for (const auto& [c, child] : node.children) {
            path.push_back(c);
            collect(*child, path);
            path.pop_back();
        }
    }

    std::vector<Suggestion> takeBestFirst() {
        std::vector<Suggestion> result;
        result.reserve(heap_.size());
        while (!heap_.empty()) {
            result.push_back(heap_.top());
            heap_.pop();
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

private:
    void offer(Weight weight, const std::string& word) {
        if (heap_.size() < k_) {
            heap_.push({word, weight});
        } else if (ranksAbove(weight, word, heap_.top().weight, heap_.top().word)) {
            heap_.pop();
            heap_.push({word, weight});
        }
        // Otherwise the candidate is rejected without copying its string.
    }

    std::size_t k_;
    BoundedMinHeap heap_;
};

}  // namespace

std::vector<Suggestion> topKCompletions(const Trie& trie, std::string_view prefix, std::size_t k) {
    if (k == 0) return {};
    const Trie::Node* start = trie.find(prefix);
    if (start == nullptr) return {};

    TopKCollector collector(k);
    std::string path(prefix);
    collector.collect(*start, path);
    return collector.takeBestFirst();
}

}  // namespace seek
