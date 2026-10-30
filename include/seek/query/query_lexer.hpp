#pragma once

#include <string_view>

#include "seek/query/query_token.hpp"
#include "seek/text/tokenizer.hpp"

namespace seek {

// Splits query text into tokens.
//
//   "Distributed AND (key-value OR lsm)"
//     -> Word(distributed) And LeftParen Word(key) Word(value) Or Word(lsm) RightParen End
//
// Text is split on spaces and parentheses. Each piece is either an operator (upper-case AND / OR)
// or is run through the document Tokenizer, so query words are normalized exactly like document
// words. That is why "key-value" becomes two words.
class QueryLexer {
public:
    explicit QueryLexer(const Tokenizer& tokenizer) : tokenizer_(tokenizer) {}

    [[nodiscard]] TokenStream lex(std::string_view text) const;

private:
    void addPiece(std::string_view piece, std::vector<Token>& tokens) const;

    const Tokenizer& tokenizer_;
};

}  // namespace seek
