#include "seek/search_engine.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace seek {
namespace {

class SearchEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine_.addDocument("Distributed consensus using Raft and an LSM storage engine");
        engine_.addDocument("High performance distributed database with a B+ tree index");
        engine_.addDocument("Kernel packet filtering using eBPF and XDP");
        engine_.addDocument("Distributed key-value storage engine with a skip list");
    }

    SearchEngine engine_;
};

TEST_F(SearchEngineTest, BooleanSearchIsCaseInsensitive) {
    EXPECT_EQ(engine_.search("DISTRIBUTED AND Storage"), (std::vector<DocId>{0, 3}));
    EXPECT_EQ(engine_.search("raft OR ebpf"), (std::vector<DocId>{0, 2}));
}

TEST_F(SearchEngineTest, MalformedQueryThrows) {
    EXPECT_THROW((void)engine_.search("storage AND"), QueryError);
}

TEST_F(SearchEngineTest, AutocompleteRanksByCorpusFrequency) {
    const auto suggestions = engine_.autocomplete("Di", 5);
    ASSERT_EQ(suggestions.size(), 1u);
    EXPECT_EQ(suggestions[0], (Suggestion{"distributed", 3}));

    const auto engineOrEbpf = engine_.autocomplete("e", 2);
    ASSERT_EQ(engineOrEbpf.size(), 2u);
    EXPECT_EQ(engineOrEbpf[0], (Suggestion{"engine", 2}));
}

TEST_F(SearchEngineTest, DidYouMeanCorrectsTyposAndRespectsLimit) {
    const auto matches = engine_.didYouMean("Distrbuted");
    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].word, "distributed");
    EXPECT_EQ(matches[0].distance, 1u);

    EXPECT_LE(engine_.didYouMean("a", 2, 3).size(), 3u);
}

TEST_F(SearchEngineTest, DocumentsAndStats) {
    EXPECT_EQ(engine_.documentCount(), 4u);
    EXPECT_EQ(engine_.document(2), "Kernel packet filtering using eBPF and XDP");
    EXPECT_THROW((void)engine_.document(99), std::out_of_range);
    EXPECT_TRUE(engine_.hasTerm("RAFT"));
    EXPECT_FALSE(engine_.hasTerm("paxos"));
}

}  // namespace
}  // namespace seek
