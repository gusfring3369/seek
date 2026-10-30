#pragma once

#include "seek/query/composite_node.hpp"

namespace seek {

// Matches documents matched by *any* child.
class OrNode final : public CompositeNode {
public:
    using CompositeNode::CompositeNode;

    [[nodiscard]] ResultSet evaluate(const QueryContext& context) const override;
    [[nodiscard]] std::string toString() const override { return joinChildren(" OR "); }
};

}  // namespace seek
