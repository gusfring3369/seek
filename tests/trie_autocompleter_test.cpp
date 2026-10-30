#include "seek/autocomplete/trie_autocompleter.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <map>
#include <random>

using namespace std;
using namespace seek;

class TrieAutocompleterTest : public ::testing::Test {
protected:
    void SetUp() override {
        trie_.insert("algorithm", 450);
        trie_.insert("algorithms", 820);
        trie_.insert("algebra", 120);
        trie_.insert("algo", 610);
        trie_.insert("alligator", 90);
        trie_.insert("alpha", 340);
    }

    Trie trie_;
    TrieAutocompleter autocompleter_{trie_};
};

TEST_F(TrieAutocompleterTest, ReturnsTopKByWeight) {
    const vector<Suggestion> expected = {{"algorithms", 820}, {"algo", 610}, {"algorithm", 450}};
    EXPECT_EQ(autocompleter_.complete("alg", 3), expected);
}

TEST_F(TrieAutocompleterTest, IncludesThePrefixItselfWhenItIsAWord) {
    const vector<Suggestion> result = autocompleter_.complete("algo", 10);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[1].word, "algo");
}

TEST_F(TrieAutocompleterTest, ReturnsFewerThanKWhenThereAreFewerMatches) {
    EXPECT_EQ(autocompleter_.complete("alp", 5).size(), 1u);
}

TEST_F(TrieAutocompleterTest, UnknownPrefixOrZeroKGivesNothing) {
    EXPECT_TRUE(autocompleter_.complete("xyz", 5).empty());
    EXPECT_TRUE(autocompleter_.complete("al", 0).empty());
}

// Property test: the heap-based answer equals "collect every match, sort, keep k".
TEST(TrieAutocompleterPropertyTest, MatchesSortingEveryMatch) {
    mt19937 rng(42);
    uniform_int_distribution<int> letter('a', 'd');
    uniform_int_distribution<size_t> length(1, 6);
    uniform_int_distribution<Weight> weight(1, 20);

    Trie trie;
    map<string, Weight> reference;
    for (int i = 0; i < 2000; ++i) {
        string word;
        for (size_t n = length(rng); n > 0; --n) word.push_back(static_cast<char>(letter(rng)));
        const Weight w = weight(rng);
        trie.insert(word, w);
        reference[word] += w;
    }

    const TrieAutocompleter autocompleter(trie);
    for (const string prefix : {"", "a", "b", "ab", "dca", "ccc"}) {
        for (size_t k : {1u, 5u, 50u}) {
            vector<Suggestion> expected;
            for (const auto& [word, w] : reference) {
                if (word.starts_with(prefix)) expected.push_back({word, w});
            }
            sort(expected.begin(), expected.end(),
                 [](const Suggestion& a, const Suggestion& b) { return a.ranksAbove(b); });
            if (expected.size() > k) expected.resize(k);
            EXPECT_EQ(autocompleter.complete(prefix, k), expected) << "prefix=" << prefix;
        }
    }
}
