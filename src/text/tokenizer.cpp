#include "seek/text/tokenizer.hpp"

#include <cctype>

using namespace std;

namespace seek {

vector<string> Tokenizer::tokenize(string_view text) const {
    vector<string> terms;
    string current;
    for (char c : text) {
        if (isTermCharacter(c)) {
            current.push_back(toLower(c));
        } else if (!current.empty()) {
            terms.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) terms.push_back(current);
    return terms;
}

string Tokenizer::normalize(string_view word) const {
    string lowered(word);
    for (char& c : lowered) c = toLower(c);
    return lowered;
}

// <cctype> functions need the char converted to unsigned char first, or bytes >= 128 are UB.
bool Tokenizer::isTermCharacter(char c) { return isalnum(static_cast<unsigned char>(c)) != 0; }

char Tokenizer::toLower(char c) {
    return static_cast<char>(tolower(static_cast<unsigned char>(c)));
}

}  // namespace seek
