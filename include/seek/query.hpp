#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "seek/inverted_index.hpp"
#include "seek/types.hpp"

namespace seek {

// Boolean query syntax tree. And/Or nodes always have at least two children.
struct Query {
    enum class Kind { Term, And, Or };

    Kind kind = Kind::Term;
    std::string term;             // Kind::Term only, already normalized
    std::vector<Query> children;  // Kind::And / Kind::Or only

    friend bool operator==(const Query&, const Query&) = default;
};

class QueryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Parses a boolean query. Words are normalized with the same tokenizer used for documents.
//
//   query  := and ( "OR" and )*
//   and    := factor ( ["AND"] factor )*      adjacent terms are ANDed
//   factor := WORD | "(" query ")"
//
// AND binds tighter than OR. Only upper-case AND / OR are operators; "and" is a plain word.
// Throws QueryError on malformed input.
[[nodiscard]] Query parseQuery(std::string_view text);

// Canonical, fully parenthesized form, e.g. "(distributed AND (storage OR database))".
[[nodiscard]] std::string toString(const Query& query);

// Evaluates a query against the index and returns the matching ids in ascending order.
//
// Term lists are read in place rather than copied. AND operands are intersected smallest
// first and stop early once the result is empty, so a rare term keeps the whole query cheap.
[[nodiscard]] PostingList execute(const Query& query, const InvertedIndex& index);

}  // namespace seek
