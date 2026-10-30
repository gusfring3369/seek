#include "seek/autocomplete/cached_autocompleter.hpp"

#include <gtest/gtest.h>

#include <random>
#include <stdexcept>

#include "seek/autocomplete/trie_autocompleter.hpp"

using namespace std;
using namespace seek;

// Inserts into the trie and reports the new weight to the cache, as the SearchEngine does.
class CachedAutocompleterTest : public ::testing::Test {
protected:
    void insert(const string& word, Weight weight) {
        cached_.onWeightChanged(word, trie_.insert(word, weight));
    }

    Trie trie_;
    TrieAutocompleter trieWalk_{trie_};
    CachedAutocompleter cached_{trieWalk_, 2, 2};  // prefixes up to 2 chars, k up to 2
};

TEST_F(CachedAutocompleterTest, AnswersShortPrefixesFromTheCache) {
    insert("apple", 5);
    insert("apricot", 9);
    insert("avocado", 7);

    EXPECT_TRUE(cached_.covers("ap", 2));
    EXPECT_EQ(cached_.complete("a", 2), (vector<Suggestion>{{"apricot", 9}, {"avocado", 7}}));
    EXPECT_EQ(cached_.complete("ap", 2), (vector<Suggestion>{{"apricot", 9}, {"apple", 5}}));
    EXPECT_EQ(cached_.complete("ap", 1), (vector<Suggestion>{{"apricot", 9}}));
    EXPECT_TRUE(cached_.complete("zz", 2).empty());
}

TEST_F(CachedAutocompleterTest, RaisingAWeightMovesItUpAndCanEvictAnother) {
    insert("ant", 1);
    insert("ape", 2);
    insert("arc", 3);  // pushes "ant" out of the top 2
    insert("ape", 8);  // "ape" is now 10
    EXPECT_EQ(cached_.complete("a", 2), (vector<Suggestion>{{"ape", 10}, {"arc", 3}}));
}

TEST_F(CachedAutocompleterTest, ForwardsWhatItDoesNotCoverToTheFallback) {
    insert("abcd", 4);
    insert("abce", 6);
    insert("abcf", 5);
    EXPECT_FALSE(cached_.covers("abc", 2));  // prefix too long
    EXPECT_FALSE(cached_.covers("a", 3));    // k too large
    EXPECT_EQ(cached_.complete("abc", 2), trieWalk_.complete("abc", 2));
    EXPECT_EQ(cached_.complete("a", 3), trieWalk_.complete("a", 3));
}

TEST(CachedAutocompleterConstructionTest, RejectsZeroCapacity) {
    Trie trie;
    TrieAutocompleter trieWalk(trie);
    EXPECT_THROW(CachedAutocompleter(trieWalk, 3, 0), invalid_argument);
}

// Property test: after thousands of random weight increases, every cached answer equals the
// answer of a full trie walk.
TEST(CachedAutocompleterPropertyTest, AlwaysAgreesWithTheTrieWalk) {
    constexpr size_t kCapacity = 5;
    mt19937 rng(99);
    uniform_int_distribution<int> letter('a', 'c');
    uniform_int_distribution<size_t> length(1, 5);
    uniform_int_distribution<Weight> weight(1, 4);

    Trie trie;
    TrieAutocompleter trieWalk(trie);
    CachedAutocompleter cached(trieWalk, 3, kCapacity);
    for (int i = 0; i < 3000; ++i) {
        string word;
        for (size_t n = length(rng); n > 0; --n) word.push_back(static_cast<char>(letter(rng)));
        cached.onWeightChanged(word, trie.insert(word, weight(rng)));
    }

    for (const string prefix : {"", "a", "b", "ab", "cab", "ccc"}) {
        for (size_t k = 1; k <= kCapacity; ++k) {
            ASSERT_TRUE(cached.covers(prefix, k));
            EXPECT_EQ(cached.complete(prefix, k), trieWalk.complete(prefix, k))
                << "prefix=" << prefix << " k=" << k;
        }
    }
}
