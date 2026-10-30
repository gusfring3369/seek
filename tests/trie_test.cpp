#include "seek/trie/trie.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace std;
using namespace seek;

TEST(TrieTest, InsertAddsUpWeightsAndReturnsTheTotal) {
    Trie trie;
    EXPECT_EQ(trie.insert("data", 3), 3u);
    EXPECT_EQ(trie.insert("data", 2), 5u);
    EXPECT_EQ(trie.weightOf("data"), 5u);
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

TEST(TrieTest, WordsWithACommonPrefixShareNodes) {
    Trie trie;
    trie.insert("tea");
    trie.insert("ten");
    trie.insert("to");
    EXPECT_EQ(trie.nodeCount(), 6u);  // root, t, e, a, n, o
}

TEST(TrieTest, EmptyPrefixIsTheRoot) {
    Trie trie;
    EXPECT_EQ(trie.find(""), &trie.root());
    EXPECT_FALSE(trie.contains(""));
}

TEST(TrieTest, ChildrenAreVisitedAlphabetically) {
    Trie trie;
    for (const char* word : {"c", "a", "b"}) trie.insert(word);
    string order;
    for (const auto& [c, child] : trie.root().children()) order.push_back(c);
    EXPECT_EQ(order, "abc");
}

TEST(TrieTest, RejectsEmptyWordAndZeroWeight) {
    Trie trie;
    EXPECT_THROW(trie.insert(""), invalid_argument);
    EXPECT_THROW(trie.insert("x", 0), invalid_argument);
    EXPECT_EQ(trie.wordCount(), 0u);
}
