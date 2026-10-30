#include "seek/tokenizer.hpp"

#include <cctype>

namespace seek {
namespace {

bool isTermChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; }

char toLower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

}  // namespace

std::vector<std::string> tokenize(std::string_view text) {
    std::vector<std::string> terms;
    std::string current;
    for (char c : text) {
        if (isTermChar(c)) {
            current.push_back(toLower(c));
        } else if (!current.empty()) {
            terms.push_back(std::move(current));
            current.clear();
        }
    }
    if (!current.empty()) terms.push_back(std::move(current));
    return terms;
}

std::string toLowerAscii(std::string_view text) {
    std::string lowered(text);
    for (char& c : lowered) c = toLower(c);
    return lowered;
}

}  // namespace seek
