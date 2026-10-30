#pragma once

#include <string>

#include "seek/query/query_node.hpp"

namespace seek {

// Leaf node: matches the documents that contain one term.
class TermNode final : public QueryNode {
public:
    explicit TermNode(std::string term) : term_(std::move(term)) {}

    [[nodiscard]] const std::string& term() const noexcept { return term_; }

    [[nodiscard]] ResultSet evaluate(const QueryContext& context) const override;
    [[nodiscard]] std::string toString() const override { return term_; }
    void collectTerms(std::vector<std::string>& terms) const override { terms.push_back(term_); }

private:
    std::string term_;  // already normalized by the tokenizer
};

}  // namespace seek
