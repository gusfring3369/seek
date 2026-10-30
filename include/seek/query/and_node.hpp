#pragma once

#include "seek/query/composite_node.hpp"

namespace seek {

// Matches documents matched by *every* child.
//
// Children are intersected smallest result first, so every intermediate result is no larger
// than the rarest term's list, and evaluation stops as soon as the result becomes empty.
class AndNode final : public CompositeNode {
public:
    using CompositeNode::CompositeNode;

    [[nodiscard]] ResultSet evaluate(const QueryContext& context) const override;
    [[nodiscard]] std::string toString() const override { return joinChildren(" AND "); }
};

}  // namespace seek
