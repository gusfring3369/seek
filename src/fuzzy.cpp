#include "seek/fuzzy.hpp"

#include <algorithm>
#include <numeric>

namespace seek {
namespace {

using DpRow = std::vector<std::size_t>;

class FuzzySearcher {
public:
    FuzzySearcher(std::string_view query, std::size_t maxDistance)
        : query_(query), maxDistance_(maxDistance) {
        // Row 0: distance from the empty prefix to query[0..j) is j insertions.
        rows_.emplace_back(query_.size() + 1);
        std::iota(rows_[0].begin(), rows_[0].end(), std::size_t{0});
    }

    std::vector<FuzzyMatch> run(const Trie::Node& root) {
        for (const auto& [c, child] : root.children) visitChild(*child, c, 1);
        std::sort(matches_.begin(), matches_.end(), [](const FuzzyMatch& a, const FuzzyMatch& b) {
            if (a.distance != b.distance) return a.distance < b.distance;
            if (a.weight != b.weight) return a.weight > b.weight;
            return a.word < b.word;
        });
        return std::move(matches_);
    }

    [[nodiscard]] const FuzzyStats& stats() const noexcept { return stats_; }

private:
    // `depth` is the length of the path spelled by `node`, i.e. its DP row index.
    void visitChild(const Trie::Node& node, char c, std::size_t depth) {
        path_.push_back(c);
        visit(node, c, depth);
        path_.pop_back();
    }

    void visit(const Trie::Node& node, char c, std::size_t depth) {
        // Rows are allocated once per depth and reused across siblings. Recursing below may grow
        // rows_, so `prev` and `row` must not be used after the children are visited.
        if (rows_.size() <= depth) rows_.emplace_back(query_.size() + 1);
        const DpRow& prev = rows_[depth - 1];
        DpRow& row = rows_[depth];

        row[0] = depth;  // delete every character of the path
        std::size_t rowMin = row[0];
        for (std::size_t j = 1; j <= query_.size(); ++j) {
            const std::size_t insertCost = row[j - 1] + 1;
            const std::size_t deleteCost = prev[j] + 1;
            const std::size_t substituteCost = prev[j - 1] + (query_[j - 1] == c ? 0 : 1);
            row[j] = std::min({insertCost, deleteCost, substituteCost});
            rowMin = std::min(rowMin, row[j]);
        }
        ++stats_.nodesVisited;
        stats_.dpCellsComputed += query_.size();

        const std::size_t distance = row[query_.size()];
        if (node.isWord() && distance <= maxDistance_) {
            matches_.push_back({path_, distance, node.weight});
        }

        if (rowMin > maxDistance_) return;  // prune the whole subtree
        for (const auto& [next, child] : node.children) visitChild(*child, next, depth + 1);
    }

    std::string_view query_;
    std::size_t maxDistance_;
    std::vector<DpRow> rows_;
    std::string path_;
    std::vector<FuzzyMatch> matches_;
    FuzzyStats stats_;
};

}  // namespace

std::vector<FuzzyMatch> fuzzySearch(const Trie& trie, std::string_view query,
                                    std::size_t maxDistance, FuzzyStats* stats) {
    FuzzySearcher searcher(query, maxDistance);
    std::vector<FuzzyMatch> matches = searcher.run(trie.root());
    if (stats != nullptr) *stats = searcher.stats();
    return matches;
}

}  // namespace seek
