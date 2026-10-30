#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace seek {

// One lexical unit of a query: a word, an operator, or a parenthesis.
struct Token {
    enum class Type { Word, And, Or, LeftParen, RightParen, End };

    Type type = Type::End;
    std::string text;  // the normalized word, for Type::Word only

    // Human-readable form for error messages, e.g. "'raft'" or "end of query".
    [[nodiscard]] std::string describe() const;
};

// Read cursor over a list of tokens. The list always ends with an End token, and the cursor
// never moves past it, so peek() is always safe.
class TokenStream {
public:
    explicit TokenStream(std::vector<Token> tokens);

    [[nodiscard]] const Token& peek() const { return tokens_[position_]; }

    // Returns the current token and moves to the next one.
    Token next();

    // If the current token has type `type`, consumes it and returns true.
    bool accept(Token::Type type);

private:
    std::vector<Token> tokens_;
    std::size_t position_ = 0;
};

}  // namespace seek
