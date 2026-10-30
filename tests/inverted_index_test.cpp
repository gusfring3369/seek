#include "seek/inverted_index.hpp"

#include <gtest/gtest.h>

namespace seek {
namespace {

using Terms = std::vector<std::string>;

TEST(InvertedIndexTest, AssignsSequentialIds) {
    InvertedIndex index;
    EXPECT_EQ(index.add(Terms{"a"}), 0u);
    EXPECT_EQ(index.add(Terms{"b"}), 1u);
    EXPECT_EQ(index.add(Terms{}), 2u);
    EXPECT_EQ(index.documentCount(), 3u);
}

TEST(InvertedIndexTest, PostingListsAreSortedAndDeduplicated) {
    InvertedIndex index;
    index.add(Terms{"raft", "log", "raft"});
    index.add(Terms{"log"});
    index.add(Terms{"raft", "raft", "raft"});
    EXPECT_EQ(index.postings("raft"), (PostingList{0, 2}));
    EXPECT_EQ(index.postings("log"), (PostingList{0, 1}));
    EXPECT_EQ(index.termCount(), 2u);
}

TEST(InvertedIndexTest, UnknownTermHasEmptyPostings) {
    InvertedIndex index;
    index.add(Terms{"x"});
    EXPECT_TRUE(index.postings("missing").empty());
}

}  // namespace
}  // namespace seek
