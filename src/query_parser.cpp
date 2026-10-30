#include <cctype>
#include <cstddef>
#include <utility>

#include "seek/query.hpp"
#include "seek/tokenizer.hpp"

namespace seek {
namespace {

struct Token {
    enum class Kind { Word, And, Or, LeftParen, RightParen, End };

    Kind kind;
    std::string text;  // normalized term, for Kind::Word
};

std::string describe(const Token& token) {
    switch (token.kind) {
        case Token::Kind::Word:
            return "'" + token.text + "'";
        case Token::Kind::And:
            return "AND";
        case Token::Kind::Or:
            return "OR";
        case Token::Kind::LeftParen:
            return "'('";
        case Token::Kind::RightParen:
            return "')'";
        case Token::Kind::End:
            return "end of query";
    }
    return "?";
}

bool isSpace(char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }
bool isParen(char c) { return c == '(' || c == ')'; }

// Splits on whitespace and parentheses. Each remaining chunk is an operator or is normalized
// by the document tokenizer, so "Key-Value" becomes two Word tokens (an implicit AND).
std::vector<Token> lex(std::string_view text) {
    std::vector<Token> tokens;
    std::size_t i = 0;
    while (i < text.size()) {
        const char c = text[i];
        if (isSpace(c)) {
            ++i;
        } else if (isParen(c)) {
            tokens.push_back({c == '(' ? Token::Kind::LeftParen : Token::Kind::RightParen, {}});
            ++i;
        } else {
            const std::size_t start = i;
            while (i < text.size() && !isSpace(text[i]) && !isParen(text[i])) ++i;
            const std::string_view chunk = text.substr(start, i - start);
            if (chunk == "AND") {
                tokens.push_back({Token::Kind::And, {}});
            } else if (chunk == "OR") {
                tokens.push_back({Token::Kind::Or, {}});
            } else {
                for (std::string& term : tokenize(chunk)) {
                    tokens.push_back({Token::Kind::Word, std::move(term)});
                }
            }
        }
    }
    tokens.push_back({Token::Kind::End, {}});
    return tokens;
}

// Builds an n-ary node, flattening same-kind children: a AND (b AND c) -> AND(a, b, c).
Query combine(Query::Kind kind, std::vector<Query> operands) {
    if (operands.size() == 1) return std::move(operands.front());
    Query node{kind, {}, {}};
    for (Query& operand : operands) {
        if (operand.kind == kind) {
            for (Query& grandchild : operand.children) {
                node.children.push_back(std::move(grandchild));
            }
        } else {
            node.children.push_back(std::move(operand));
        }
    }
    return node;
}

// Recursive-descent parser; one method per grammar rule.
class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    Query parse() {
        if (peek().kind == Token::Kind::End) throw QueryError("empty query");
        Query query = parseOr();
        if (peek().kind != Token::Kind::End) fail("unexpected " + describe(peek()));
        return query;
    }

private:
    Query parseOr() {
        std::vector<Query> operands;
        operands.push_back(parseAnd());
        while (accept(Token::Kind::Or)) operands.push_back(parseAnd());
        return combine(Query::Kind::Or, std::move(operands));
    }

    Query parseAnd() {
        std::vector<Query> operands;
        operands.push_back(parseFactor());
        while (true) {
            if (accept(Token::Kind::And) || startsFactor(peek())) {
                operands.push_back(parseFactor());
            } else {
                break;
            }
        }
        return combine(Query::Kind::And, std::move(operands));
    }

    Query parseFactor() {
        if (peek().kind == Token::Kind::Word) {
            return Query{Query::Kind::Term, std::move(tokens_[pos_++].text), {}};
        }
        if (accept(Token::Kind::LeftParen)) {
            Query inner = parseOr();
            if (!accept(Token::Kind::RightParen)) {
                fail("expected ')' but found " + describe(peek()));
            }
            return inner;
        }
        fail("expected a word or '(' but found " + describe(peek()));
    }

    static bool startsFactor(const Token& token) {
        return token.kind == Token::Kind::Word || token.kind == Token::Kind::LeftParen;
    }

    [[nodiscard]] const Token& peek() const { return tokens_[pos_]; }

    bool accept(Token::Kind kind) {
        if (peek().kind != kind) return false;
        ++pos_;
        return true;
    }

    [[noreturn]] static void fail(const std::string& message) { throw QueryError(message); }

    std::vector<Token> tokens_;  // always ends with an End token
    std::size_t pos_ = 0;
};

}  // namespace

Query parseQuery(std::string_view text) { return Parser(lex(text)).parse(); }

std::string toString(const Query& query) {
    if (query.kind == Query::Kind::Term) return query.term;
    const std::string separator = query.kind == Query::Kind::And ? " AND " : " OR ";
    std::string out = "(";
    for (std::size_t i = 0; i < query.children.size(); ++i) {
        if (i > 0) out += separator;
        out += toString(query.children[i]);
    }
    return out + ")";
}

}  // namespace seek
