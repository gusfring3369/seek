#include "seek/query/query_lexer.hpp"

#include <cctype>
#include <utility>

using namespace std;

namespace seek {

static bool isSpace(char c) { return isspace(static_cast<unsigned char>(c)) != 0; }
static bool isParenthesis(char c) { return c == '(' || c == ')'; }

TokenStream QueryLexer::lex(string_view text) const {
    vector<Token> tokens;
    size_t i = 0;
    while (i < text.size()) {
        if (isSpace(text[i])) {
            ++i;
        } else if (isParenthesis(text[i])) {
            tokens.push_back(
                {text[i] == '(' ? Token::Type::LeftParen : Token::Type::RightParen, {}});
            ++i;
        } else {
            const size_t start = i;
            while (i < text.size() && !isSpace(text[i]) && !isParenthesis(text[i])) ++i;
            addPiece(text.substr(start, i - start), tokens);
        }
    }
    tokens.push_back({Token::Type::End, {}});
    return TokenStream(std::move(tokens));
}

void QueryLexer::addPiece(string_view piece, vector<Token>& tokens) const {
    if (piece == "AND") {
        tokens.push_back({Token::Type::And, {}});
    } else if (piece == "OR") {
        tokens.push_back({Token::Type::Or, {}});
    } else {
        for (string& word : tokenizer_.tokenize(piece)) {
            tokens.push_back({Token::Type::Word, std::move(word)});
        }
    }
}

}  // namespace seek
