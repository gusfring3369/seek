#include "seek/trie.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace seek {
namespace {

TEST(TrieTest, InsertAccumulatesWeight) {
    Trie trie;
    trie.insert("data", 3);
    trie.insert("data", 2);
    EXPECT_EQ(trie.weight("data"), 5u);
    EXPECT_EQ(trie.wordCount(), 1u);
}

TEST(TrieTest, PrefixesAreNotWordsUntilInserted) {
    Trie trie;
    trie.insert("database");
    EXPECT_FALSE(trie.contains("data"));
    EXPECT_NE(trie.find("data"), nullptr);
    EXPECT_EQ(trie.find("dog"), nullptr);

    trie.insert("data");
    EXPECT_TRUE(trie.contains("data"));
    EXPECT_EQ(trie.wordCount(), 2u);
}

TEST(TrieTest, SharedPrefixesShareNodes) {
    Trie trie;
    trie.insert("tea");
    trie.insert("ten");
    trie.insert("to");
    // root + t + e + a + n + o
    EXPECT_EQ(trie.nodeCount(), 6u);
}

TEST(TrieTest, EmptyPrefixFindsRoot) {
    Trie trie;
    EXPECT_EQ(trie.find(""), &trie.root());
    EXPECT_FALSE(trie.contains(""));
}

TEST(TrieTest, RejectsEmptyWordAndZeroWeight) {
    Trie trie;
    EXPECT_THROW(trie.insert(""), std::invalid_argument);
    EXPECT_THROW(trie.insert("x", 0), std::invalid_argument);
    EXPECT_EQ(trie.wordCount(), 0u);
}

TEST(TrieTest, IsMovable) {
    Trie trie;
    trie.insert("alpha");
    Trie moved = std::move(trie);
    EXPECT_TRUE(moved.contains("alpha"));
}

}  // namespace
}  // namespace seek
