#pragma once

#include <string>
#include <vector>

#include "seek/index/intersection_strategy.hpp"
#include "seek/index/inverted_index.hpp"
#include "seek/query/result_set.hpp"

namespace seek {

// Everything a query needs while it runs.
struct QueryContext {
    const InvertedIndex& index;
    const IntersectionStrategy& intersection;
};

// A node of a parsed boolean query. Queries form a tree (Composite pattern):
//
//   "distributed AND (storage OR database)"
//
//     AndNode
//      |-- TermNode("distributed")
//      `-- OrNode
//           |-- TermNode("storage")
//           `-- TermNode("database")
//
// Calling evaluate() on the root runs the whole query: each node evaluates its children and
// combines their results (Interpreter pattern).
class QueryNode {
public:
    QueryNode() = default;
    QueryNode(const QueryNode&) = delete;  // polymorphic: no copies (avoids slicing)
    QueryNode& operator=(const QueryNode&) = delete;
    virtual ~QueryNode() = default;

    // The documents this subtree matches, in increasing id order.
    [[nodiscard]] virtual ResultSet evaluate(const QueryContext& context) const = 0;

    // Fully parenthesized form, e.g. "(distributed AND (storage OR database))".
    [[nodiscard]] virtual std::string toString() const = 0;

    // Appends every term in this subtree to `terms`.
    virtual void collectTerms(std::vector<std::string>& terms) const = 0;
};

}  // namespace seek
