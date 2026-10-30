#include "seek/query/query_parser.hpp"

#include "seek/query/and_node.hpp"
#include "seek/query/or_node.hpp"
#include "seek/query/term_node.hpp"

using namespace std;

namespace seek {

unique_ptr<QueryNode> QueryParser::parse(string_view text) const {
    TokenStream tokens = lexer_.lex(text);
    if (tokens.peek().type == Token::Type::End) throw QueryError("empty query");

    unique_ptr<QueryNode> query = parseOr(tokens);
    if (tokens.peek().type != Token::Type::End) {
        throw QueryError("unexpected " + tokens.peek().describe());
    }
    return query;
}

unique_ptr<QueryNode> QueryParser::parseOr(TokenStream& tokens) {
    Operands operands;
    operands.push_back(parseAnd(tokens));
    while (tokens.accept(Token::Type::Or)) operands.push_back(parseAnd(tokens));
    return combine<OrNode>(std::move(operands));
}

unique_ptr<QueryNode> QueryParser::parseAnd(TokenStream& tokens) {
    Operands operands;
    operands.push_back(parseFactor(tokens));
    // An explicit AND, or simply another word / "(" right after (implicit AND).
    while (tokens.accept(Token::Type::And) || startsFactor(tokens.peek())) {
        operands.push_back(parseFactor(tokens));
    }
    return combine<AndNode>(std::move(operands));
}

unique_ptr<QueryNode> QueryParser::parseFactor(TokenStream& tokens) {
    if (tokens.peek().type == Token::Type::Word) return make_unique<TermNode>(tokens.next().text);

    if (tokens.accept(Token::Type::LeftParen)) {
        unique_ptr<QueryNode> inner = parseOr(tokens);
        if (!tokens.accept(Token::Type::RightParen)) {
            throw QueryError("expected ')' but found " + tokens.peek().describe());
        }
        return inner;
    }
    throw QueryError("expected a word or '(' but found " + tokens.peek().describe());
}

template <typename NodeType>
unique_ptr<QueryNode> QueryParser::combine(Operands operands) {
    if (operands.size() == 1) return std::move(operands.front());

    Operands flattened;
    for (unique_ptr<QueryNode>& operand : operands) {
        if (auto* sameKind = dynamic_cast<NodeType*>(operand.get())) {
            for (auto& grandchild : sameKind->releaseChildren()) {
                flattened.push_back(std::move(grandchild));
            }
        } else {
            flattened.push_back(std::move(operand));
        }
    }
    return make_unique<NodeType>(std::move(flattened));
}

bool QueryParser::startsFactor(const Token& token) {
    return token.type == Token::Type::Word || token.type == Token::Type::LeftParen;
}

}  // namespace seek
