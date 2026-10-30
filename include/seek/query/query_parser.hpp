#pragma once

#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "seek/query/query_lexer.hpp"
#include "seek/query/query_node.hpp"

namespace seek {

// Thrown for malformed queries, e.g. "storage AND" or "(raft".
class QueryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Turns query text into a tree of QueryNodes using recursive descent: one method per rule of
// the grammar below, each calling the rules it is built from.
//
//   query  := and ( "OR" and )*              OR has the lowest precedence
//   and    := factor ( ["AND"] factor )*     AND binds tighter; "a b" means "a AND b"
//   factor := WORD | "(" query ")"
//
// Only upper-case AND / OR are operators, so "rock and roll" searches for three words.
class QueryParser {
public:
    explicit QueryParser(const Tokenizer& tokenizer) : lexer_(tokenizer) {}

    // Throws QueryError if `text` is not a valid query.
    [[nodiscard]] std::unique_ptr<QueryNode> parse(std::string_view text) const;

private:
    using Operands = std::vector<std::unique_ptr<QueryNode>>;

    static std::unique_ptr<QueryNode> parseOr(TokenStream& tokens);
    static std::unique_ptr<QueryNode> parseAnd(TokenStream& tokens);
    static std::unique_ptr<QueryNode> parseFactor(TokenStream& tokens);

    // Wraps operands in a NodeType (AndNode or OrNode), flattening nested NodeTypes.
    // A single operand is returned as is.
    template <typename NodeType>
    static std::unique_ptr<QueryNode> combine(Operands operands);

    static bool startsFactor(const Token& token);

    QueryLexer lexer_;
};

}  // namespace seek
