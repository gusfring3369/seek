#include "seek/query/query_token.hpp"

#include <utility>

using namespace std;

namespace seek {

string Token::describe() const {
    switch (type) {
        case Type::Word:
            return "'" + text + "'";
        case Type::And:
            return "AND";
        case Type::Or:
            return "OR";
        case Type::LeftParen:
            return "'('";
        case Type::RightParen:
            return "')'";
        case Type::End:
            return "end of query";
    }
    return "?";
}

TokenStream::TokenStream(vector<Token> tokens) : tokens_(std::move(tokens)) {
    if (tokens_.empty() || tokens_.back().type != Token::Type::End) {
        tokens_.push_back({Token::Type::End, {}});
    }
}

Token TokenStream::next() {
    Token current = tokens_[position_];
    if (current.type != Token::Type::End) ++position_;
    return current;
}

bool TokenStream::accept(Token::Type type) {
    if (peek().type != type) return false;
    next();
    return true;
}

}  // namespace seek
