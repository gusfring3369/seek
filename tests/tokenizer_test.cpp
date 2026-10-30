#include "seek/text/tokenizer.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace seek;

using Terms = vector<string>;

TEST(TokenizerTest, LowercasesAndSplitsOnNonAlphanumerics) {
    EXPECT_EQ(Tokenizer().tokenize("Key-Value B+Tree, v2!"),
              (Terms{"key", "value", "b", "tree", "v2"}));
}

TEST(TokenizerTest, IgnoresLeadingTrailingAndRepeatedSeparators) {
    EXPECT_EQ(Tokenizer().tokenize("  --hello   world--  "), (Terms{"hello", "world"}));
}

TEST(TokenizerTest, EmptyAndSeparatorOnlyInputGiveNoTerms) {
    EXPECT_TRUE(Tokenizer().tokenize("").empty());
    EXPECT_TRUE(Tokenizer().tokenize(" ,.;!? ").empty());
}

TEST(TokenizerTest, NonAsciiBytesAreSeparators) {
    EXPECT_EQ(Tokenizer().tokenize("caf\xC3\xA9 ok"), (Terms{"caf", "ok"}));
}

TEST(TokenizerTest, NormalizeOnlyLowercasesLetters) {
    EXPECT_EQ(Tokenizer().normalize("AbC-12 xY"), "abc-12 xy");
}
