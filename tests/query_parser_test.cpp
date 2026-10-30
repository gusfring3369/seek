#include "seek/query/query_parser.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace seek;

class QueryParserTest : public ::testing::Test {
protected:
    // Parses and prints back in fully parenthesized form, which shows the tree's shape.
    string parsed(string_view text) const { return parser_.parse(text)->toString(); }

    Tokenizer tokenizer_;
    QueryParser parser_{tokenizer_};
};

TEST_F(QueryParserTest, SingleWordIsNormalized) { EXPECT_EQ(parsed("Raft"), "raft"); }

TEST_F(QueryParserTest, AndBindsTighterThanOr) {
    EXPECT_EQ(parsed("a OR b AND c"), "(a OR (b AND c))");
    EXPECT_EQ(parsed("a AND b OR c"), "((a AND b) OR c)");
}

TEST_F(QueryParserTest, ParenthesesOverridePrecedence) {
    EXPECT_EQ(parsed("(a OR b) AND c"), "((a OR b) AND c)");
}

TEST_F(QueryParserTest, NeighbouringWordsAreAnded) {
    EXPECT_EQ(parsed("distributed storage"), "(distributed AND storage)");
    EXPECT_EQ(parsed("key-value OR lsm"), "((key AND value) OR lsm)");
}

TEST_F(QueryParserTest, NestedSameOperatorsAreFlattened) {
    EXPECT_EQ(parsed("a AND (b AND c) AND d"), "(a AND b AND c AND d)");
    EXPECT_EQ(parsed("a OR (b OR c)"), "(a OR b OR c)");
}

TEST_F(QueryParserTest, LowercaseOperatorsAreOrdinaryWords) {
    EXPECT_EQ(parsed("rock and roll"), "(rock AND and AND roll)");
}

TEST_F(QueryParserTest, CollectsAllTerms) {
    vector<string> terms;
    parser_.parse("a AND (b OR c)")->collectTerms(terms);
    EXPECT_EQ(terms, (vector<string>{"a", "b", "c"}));
}

TEST_F(QueryParserTest, RejectsMalformedQueries) {
    for (const char* bad :
         {"", "   ", "AND", "a AND", "OR b", "(a", "a)", "()", "a AND OR b", "+++"}) {
        EXPECT_THROW((void)parser_.parse(bad), QueryError) << "query: '" << bad << "'";
    }
}
