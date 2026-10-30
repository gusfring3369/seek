#include "seek/tokenizer.hpp"

#include <gtest/gtest.h>

namespace seek {
namespace {

using Terms = std::vector<std::string>;

TEST(TokenizerTest, LowercasesAndSplitsOnNonAlphanumerics) {
    EXPECT_EQ(tokenize("Key-Value B+Tree, v2!"), (Terms{"key", "value", "b", "tree", "v2"}));
}

TEST(TokenizerTest, IgnoresLeadingTrailingAndRepeatedSeparators) {
    EXPECT_EQ(tokenize("  --hello   world--  "), (Terms{"hello", "world"}));
}

TEST(TokenizerTest, EmptyAndSeparatorOnlyInputYieldNoTerms) {
    EXPECT_TRUE(tokenize("").empty());
    EXPECT_TRUE(tokenize(" ,.;!? ").empty());
}

TEST(TokenizerTest, TreatsNonAsciiBytesAsSeparators) {
    EXPECT_EQ(tokenize("caf\xC3\xA9 ok"), (Terms{"caf", "ok"}));
}

TEST(TokenizerTest, ToLowerAsciiOnlyChangesLetters) {
    EXPECT_EQ(toLowerAscii("AbC-12 xY"), "abc-12 xy");
}

}  // namespace
}  // namespace seek
