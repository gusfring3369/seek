#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "seek/query/query_node.hpp"

namespace seek {

// Base class for operators that combine two or more child queries (AndNode, OrNode).
class CompositeNode : public QueryNode {
public:
    using Children = std::vector<std::unique_ptr<QueryNode>>;

    explicit CompositeNode(Children children) : children_(std::move(children)) {}

    [[nodiscard]] const Children& children() const noexcept { return children_; }

    // Hands the children over to the caller. The parser uses this to flatten nested operators of
    // the same kind: "a AND (b AND c)" becomes one AndNode with three children.
    [[nodiscard]] Children releaseChildren() { return std::move(children_); }

    void collectTerms(std::vector<std::string>& terms) const override;

protected:
    [[nodiscard]] std::vector<ResultSet> evaluateChildren(const QueryContext& context) const;

    // e.g. "(a AND b AND c)" for separator " AND ".
    [[nodiscard]] std::string joinChildren(std::string_view separator) const;

private:
    Children children_;
};

}  // namespace seek
