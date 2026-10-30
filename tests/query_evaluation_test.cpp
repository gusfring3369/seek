#include <gtest/gtest.h>

#include "seek/index/adaptive_intersection.hpp"
#include "seek/query/query_parser.hpp"

using namespace std;
using namespace seek;

class QueryEvaluationTest : public ::testing::Test {
protected:
    void SetUp() override {
        index_.add(vector<string>{"distributed", "storage"});              // doc 0
        index_.add(vector<string>{"distributed", "database"});             // doc 1
        index_.add(vector<string>{"kernel", "storage"});                   // doc 2
        index_.add(vector<string>{"distributed", "storage", "database"});  // doc 3
    }

    PostingList run(string_view query) const {
        const QueryContext context{index_, intersection_};
        return parser_.parse(query)->evaluate(context).toPostingList();
    }

    InvertedIndex index_;
    AdaptiveIntersection intersection_;
    Tokenizer tokenizer_;
    QueryParser parser_{tokenizer_};
};

TEST_F(QueryEvaluationTest, Term) { EXPECT_EQ(run("storage"), (PostingList{0, 2, 3})); }

TEST_F(QueryEvaluationTest, And) {
    EXPECT_EQ(run("distributed AND storage"), (PostingList{0, 3}));
    EXPECT_EQ(run("distributed storage database"), (PostingList{3}));
}

TEST_F(QueryEvaluationTest, Or) { EXPECT_EQ(run("kernel OR database"), (PostingList{1, 2, 3})); }

TEST_F(QueryEvaluationTest, Nested) {
    EXPECT_EQ(run("distributed AND (storage OR database)"), (PostingList{0, 1, 3}));
    EXPECT_EQ(run("(kernel AND storage) OR (database AND distributed)"), (PostingList{1, 2, 3}));
}

TEST_F(QueryEvaluationTest, UnknownTerm) {
    EXPECT_TRUE(run("distributed AND nonexistent").empty());
    EXPECT_EQ(run("distributed OR nonexistent"), (PostingList{0, 1, 3}));
}
