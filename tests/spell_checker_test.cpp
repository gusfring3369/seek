#include "seek/spellcheck/spell_checker.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <map>
#include <random>

#include "seek/spellcheck/edit_distance.hpp"

using namespace std;
using namespace seek;

static vector<string> wordsOf(const vector<Correction>& corrections) {
    vector<string> words;
    for (const Correction& correction : corrections) words.push_back(correction.word);
    return words;
}

TEST(SpellCheckerTest, FindsAMissingLetter) {
    Trie trie;
    trie.insert("distributed", 10);
    trie.insert("distribute", 3);
    trie.insert("database", 8);

    const vector<Correction> corrections = SpellChecker(trie).suggest("distrbuted", 2);
    ASSERT_EQ(corrections.size(), 2u);
    EXPECT_EQ(corrections[0], (Correction{"distributed", 1, 10}));
    EXPECT_EQ(corrections[1], (Correction{"distribute", 2, 3}));
}

TEST(SpellCheckerTest, HandlesSubstitutionInsertionAndDeletion) {
    Trie trie;
    for (const char* word : {"cat", "cart", "at", "bat", "dog"}) trie.insert(word);
    const SpellChecker checker(trie);
    EXPECT_EQ(wordsOf(checker.suggest("cat", 0)), (vector<string>{"cat"}));
    EXPECT_EQ(wordsOf(checker.suggest("cat", 1)), (vector<string>{"cat", "at", "bat", "cart"}));
}

TEST(SpellCheckerTest, OrdersByDistanceThenWeightThenWord) {
    Trie trie;
    trie.insert("bat", 1);
    trie.insert("cab", 5);
    trie.insert("hat", 5);
    EXPECT_EQ(wordsOf(SpellChecker(trie).suggest("cat", 1)), (vector<string>{"cab", "hat", "bat"}));
}

TEST(SpellCheckerTest, EmptyWordMatchesShortWords) {
    Trie trie;
    for (const char* word : {"a", "ab", "abc"}) trie.insert(word);
    EXPECT_EQ(wordsOf(SpellChecker(trie).suggest("", 2)), (vector<string>{"a", "ab"}));
}

TEST(SpellCheckerTest, PruningSkipsMostOfTheTrie) {
    Trie trie;
    trie.insert("zebra");
    for (char c = 'a'; c <= 'y'; ++c) trie.insert(string(1, c) + "qqqqqqqqqqqq");

    SpellCheckStats stats;
    const vector<Correction> corrections = SpellChecker(trie).suggest("zebra", 1, &stats);
    EXPECT_EQ(wordsOf(corrections), (vector<string>{"zebra"}));
    EXPECT_LT(stats.nodesVisited, trie.nodeCount() / 4);
    EXPECT_EQ(stats.dpCellsComputed, stats.nodesVisited * 5);
}

// Property test: the pruned trie search finds exactly what checking every word finds.
TEST(SpellCheckerPropertyTest, MatchesCheckingEveryWord) {
    mt19937 rng(7);
    uniform_int_distribution<int> letter('a', 'e');
    uniform_int_distribution<size_t> length(1, 7);
    auto randomWord = [&] {
        string word;
        for (size_t n = length(rng); n > 0; --n) word.push_back(static_cast<char>(letter(rng)));
        return word;
    };

    Trie trie;
    map<string, Weight> dictionary;
    for (int i = 0; i < 1500; ++i) {
        const string word = randomWord();
        trie.insert(word);
        ++dictionary[word];
    }

    const SpellChecker checker(trie);
    for (int q = 0; q < 60; ++q) {
        const string query = randomWord();
        for (size_t maxDistance : {0u, 1u, 2u, 3u}) {
            vector<Correction> expected;
            for (const auto& [word, weight] : dictionary) {
                const size_t d = EditDistance::between(word, query);
                if (d <= maxDistance) expected.push_back({word, d, weight});
            }
            sort(expected.begin(), expected.end(), [](const Correction& a, const Correction& b) {
                if (a.distance != b.distance) return a.distance < b.distance;
                if (a.weight != b.weight) return a.weight > b.weight;
                return a.word < b.word;
            });
            EXPECT_EQ(checker.suggest(query, maxDistance), expected)
                << "query=" << query << " maxDistance=" << maxDistance;
        }
    }
}
