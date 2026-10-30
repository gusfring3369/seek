#include "seek/spellcheck/spell_checker.hpp"

#include <algorithm>

#include "seek/spellcheck/edit_distance.hpp"

using namespace std;

namespace seek {

// One depth-first search over the trie for one misspelled word.
class SpellChecker::Search {
public:
    Search(string_view target, size_t maxDistance) : target_(target), maxDistance_(maxDistance) {
        rows_.push_back(EditDistance::firstRow(target_));  // the root spells the empty string
    }

    vector<Correction> run(const TrieNode& root) {
        for (const auto& [c, child] : root.children()) visit(*child, c, 1);
        sort(corrections_.begin(), corrections_.end(), closestFirst);
        return corrections_;
    }

    [[nodiscard]] const SpellCheckStats& stats() const { return stats_; }

private:
    // `node` is reached by character `c` and spells a word of length `depth`.
    void visit(const TrieNode& node, char c, size_t depth) {
        path_.push_back(c);

        // rows_[depth] is reused by every node at this depth, so it is allocated only once.
        if (rows_.size() <= depth) rows_.emplace_back();
        const size_t smallest = EditDistance::nextRow(rows_[depth - 1], c, target_, rows_[depth]);
        ++stats_.nodesVisited;
        stats_.dpCellsComputed += target_.size();

        const size_t distance = rows_[depth].back();
        if (node.isEndOfWord() && distance <= maxDistance_) {
            corrections_.push_back({path_, distance, node.weight()});
        }

        // Pruning: nothing below can get closer than `smallest`.
        if (smallest <= maxDistance_) {
            for (const auto& [next, child] : node.children()) visit(*child, next, depth + 1);
        }

        path_.pop_back();
    }

    static bool closestFirst(const Correction& a, const Correction& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        if (a.weight != b.weight) return a.weight > b.weight;
        return a.word < b.word;
    }

    string_view target_;
    size_t maxDistance_;
    vector<EditDistance::Row> rows_;  // rows_[d] = row for the current path's first d characters
    string path_;                     // the word spelled by the current path
    vector<Correction> corrections_;
    SpellCheckStats stats_;
};

vector<Correction> SpellChecker::suggest(string_view word, size_t maxDistance,
                                         SpellCheckStats* stats) const {
    Search search(word, maxDistance);
    vector<Correction> corrections = search.run(dictionary_.root());
    if (stats != nullptr) *stats = search.stats();
    return corrections;
}

}  // namespace seek
