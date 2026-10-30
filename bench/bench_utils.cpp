#include "bench_utils.hpp"

#include <algorithm>
#include <fstream>
#include <numeric>

using namespace std;

namespace seek::bench {

LatencySummary summarize(vector<double> samples) {
    sort(samples.begin(), samples.end());
    const auto percentile = [&](double q) {
        return samples[static_cast<size_t>(q * static_cast<double>(samples.size() - 1))];
    };
    const double total = accumulate(samples.begin(), samples.end(), 0.0);
    return {total / static_cast<double>(samples.size()), percentile(0.50), percentile(0.99)};
}

static bool isLowerCaseWord(const string& word) {
    return !word.empty() &&
           all_of(word.begin(), word.end(), [](char c) { return c >= 'a' && c <= 'z'; });
}

vector<string> loadWords(const string& path) {
    ifstream file(path);
    vector<string> words;
    string line;
    while (getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        if (isLowerCaseWord(line)) words.push_back(line);
    }
    return words;
}

vector<string> syntheticWords(mt19937& rng, size_t count) {
    uniform_int_distribution<int> letter('a', 'z');
    uniform_int_distribution<size_t> length(3, 12);
    vector<string> words(count);
    for (string& word : words) {
        for (size_t n = length(rng); n > 0; --n) word.push_back(static_cast<char>(letter(rng)));
    }
    return words;
}

string withRandomTypo(string word, mt19937& rng) {
    uniform_int_distribution<int> letter('a', 'z');
    uniform_int_distribution<size_t> position(0, word.size() - 1);
    const size_t at = position(rng);
    const char randomLetter = static_cast<char>(letter(rng));
    switch (rng() % 3) {
        case 0:
            word[at] = randomLetter;
            break;
        case 1:
            word.insert(at, 1, randomLetter);
            break;
        default:
            word.erase(at, 1);
            break;
    }
    return word;
}

}  // namespace seek::bench
