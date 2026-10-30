#include "seek/query.hpp"

#include <gtest/gtest.h>

namespace seek {
namespace {

std::string parsed(std::string_view text) { return toString(parseQuery(text)); }

TEST(QueryParserTest, SingleTermIsNormalized) { EXPECT_EQ(parsed("Raft"), "raft"); }

TEST(QueryParserTest, AndBindsTighterThanOr) {
    EXPECT_EQ(parsed("a OR b AND c"), "(a OR (b AND c))");
    EXPECT_EQ(parsed("a AND b OR c"), "((a AND b) OR c)");
}

TEST(QueryParserTest, ParenthesesOverridePrecedence) {
    EXPECT_EQ(parsed("(a OR b) AND c"), "((a OR b) AND c)");
}

TEST(QueryParserTest, AdjacentWordsAreImplicitlyAnded) {
    EXPECT_EQ(parsed("distributed storage"), "(distributed AND storage)");
    EXPECT_EQ(parsed("key-value OR lsm"), "((key AND value) OR lsm)");
}

TEST(QueryParserTest, SameOperatorChainsAreFlattened) {
    EXPECT_EQ(parsed("a AND (b AND c) AND d"), "(a AND b AND c AND d)");
    EXPECT_EQ(parsed("a OR (b OR c)"), "(a OR b OR c)");
}

TEST(QueryParserTest, LowercaseOperatorsArePlainWords) {
    EXPECT_EQ(parsed("rock and roll"), "(rock AND and AND roll)");
}

TEST(QueryParserTest, RejectsMalformedQueries) {
    for (const char* bad :
         {"", "   ", "AND", "a AND", "OR b", "(a", "a)", "()", "a AND OR b", "+++"}) {
        EXPECT_THROW((void)parseQuery(bad), QueryError) << "query: '" << bad << "'";
    }
}

class QueryExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        index_.add(std::vector<std::string>{"distributed", "storage"});              // 0
        index_.add(std::vector<std::string>{"distributed", "database"});             // 1
        index_.add(std::vector<std::string>{"kernel", "storage"});                   // 2
        index_.add(std::vector<std::string>{"distributed", "storage", "database"});  // 3
    }

    PostingList run(std::string_view text) const { return execute(parseQuery(text), index_); }

    InvertedIndex index_;
};

TEST_F(QueryExecutorTest, Term) { EXPECT_EQ(run("storage"), (PostingList{0, 2, 3})); }

TEST_F(QueryExecutorTest, And) {
    EXPECT_EQ(run("distributed AND storage"), (PostingList{0, 3}));
    EXPECT_EQ(run("distributed storage database"), (PostingList{3}));
}

TEST_F(QueryExecutorTest, Or) { EXPECT_EQ(run("kernel OR database"), (PostingList{1, 2, 3})); }

TEST_F(QueryExecutorTest, Nested) {
    EXPECT_EQ(run("distributed AND (storage OR database)"), (PostingList{0, 1, 3}));
    EXPECT_EQ(run("(kernel AND storage) OR (database AND distributed)"), (PostingList{1, 2, 3}));
}

TEST_F(QueryExecutorTest, UnknownTermShortCircuitsAnd) {
    EXPECT_TRUE(run("distributed AND nonexistent").empty());
    EXPECT_EQ(run("distributed OR nonexistent"), (PostingList{0, 1, 3}));
}

}  // namespace
}  // namespace seek
