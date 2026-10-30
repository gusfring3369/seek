#include <algorithm>
#include <span>
#include <utility>

#include "seek/posting_list.hpp"
#include "seek/query.hpp"

namespace seek {
namespace {

// Result of evaluating a subquery: a term's posting list read in place, or a list computed
// by an operator. Borrowing avoids copying a large list that may only be galloped through.
class Operand {
public:
    static Operand borrow(const PostingList& list) { return Operand(&list, {}); }
    static Operand own(PostingList list) { return Operand(nullptr, std::move(list)); }

    [[nodiscard]] std::span<const DocId> ids() const {
        return borrowed_ != nullptr ? std::span<const DocId>(*borrowed_) : owned_;
    }

    [[nodiscard]] PostingList release() && {
        if (borrowed_ != nullptr) return *borrowed_;
        return std::move(owned_);
    }

private:
    Operand(const PostingList* borrowed, PostingList owned)
        : borrowed_(borrowed), owned_(std::move(owned)) {}

    const PostingList* borrowed_;  // set for a term's list; it lives in the index
    PostingList owned_;            // used when borrowed_ is null
};

Operand evaluate(const Query& query, const InvertedIndex& index);

std::vector<Operand> evaluateAll(const std::vector<Query>& queries, const InvertedIndex& index) {
    std::vector<Operand> operands;
    operands.reserve(queries.size());
    for (const Query& query : queries) operands.push_back(evaluate(query, index));
    return operands;
}

Operand evaluateAnd(const Query& query, const InvertedIndex& index) {
    std::vector<Operand> operands = evaluateAll(query.children, index);
    if (operands.empty()) return Operand::own({});

    // Smallest list first: every intermediate result is bounded by the rarest term.
    std::sort(operands.begin(), operands.end(),
              [](const Operand& a, const Operand& b) { return a.ids().size() < b.ids().size(); });
    Operand result = std::move(operands.front());
    for (std::size_t i = 1; i < operands.size() && !result.ids().empty(); ++i) {
        result = Operand::own(postings::intersect(result.ids(), operands[i].ids()));
    }
    return result;
}

Operand evaluateOr(const Query& query, const InvertedIndex& index) {
    std::vector<Operand> operands = evaluateAll(query.children, index);
    if (operands.empty()) return Operand::own({});

    Operand result = std::move(operands.front());
    for (std::size_t i = 1; i < operands.size(); ++i) {
        result = Operand::own(postings::unite(result.ids(), operands[i].ids()));
    }
    return result;
}

Operand evaluate(const Query& query, const InvertedIndex& index) {
    switch (query.kind) {
        case Query::Kind::Term:
            return Operand::borrow(index.postings(query.term));
        case Query::Kind::And:
            return evaluateAnd(query, index);
        case Query::Kind::Or:
            return evaluateOr(query, index);
    }
    return Operand::own({});
}

}  // namespace

PostingList execute(const Query& query, const InvertedIndex& index) {
    return evaluate(query, index).release();
}

}  // namespace seek
