#include "shell.hpp"

#include <format>
#include <iostream>
#include <vector>

#include "stopwatch.hpp"

using namespace std;

namespace seek::cli {

static constexpr size_t kMaxResultsShown = 10;
static constexpr size_t kMaxSnippetLength = 96;
static constexpr size_t kSuggestionCount = 5;
static constexpr size_t kMaxTypos = 2;

static string_view trim(string_view text) {
    const size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string_view::npos) return {};
    const size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

static string snippet(string_view text) {
    if (text.size() <= kMaxSnippetLength) return string(text);
    return string(text.substr(0, kMaxSnippetLength - 3)) + "...";
}

bool Shell::execute(string_view line) {
    line = trim(line);
    if (line.empty()) return true;
    if (line.front() != ':') {
        search(line);
        return true;
    }

    // ":command argument"
    const size_t space = line.find(' ');
    const string_view command = line.substr(0, space);
    const string_view argument = space == string_view::npos ? "" : trim(line.substr(space));

    if (command == ":quit" || command == ":q") return false;

    if (command == ":help" || command == ":h") {
        printHelp();
    } else if (command == ":stats") {
        printStats();
    } else if ((command == ":complete" || command == ":c") && !argument.empty()) {
        complete(argument);
    } else if ((command == ":fix" || command == ":f") && !argument.empty()) {
        fix(argument);
    } else {
        cout << "Unknown command or missing argument. Type :help.\n";
    }
    return true;
}

void Shell::printHelp() {
    cout << "Commands:\n"
            "  <query>             boolean search, e.g.  distributed AND (storage OR database)\n"
            "                      AND binds tighter than OR; words next to each other are ANDed\n"
            "  :complete <prefix>  top-5 completions by frequency          (alias :c)\n"
            "  :fix <word>         \"did you mean\" within 2 edits          (alias :f)\n"
            "  :stats              corpus statistics\n"
            "  :help               this message\n"
            "  :quit               exit                                    (or Ctrl-D)\n";
}

void Shell::search(string_view queryText) const {
    const Stopwatch timer;
    unique_ptr<QueryNode> query;
    try {
        query = engine_.parse(queryText);
    } catch (const QueryError& error) {
        cout << "Query error: " << error.what() << '\n';
        return;
    }
    const vector<DocId> results = engine_.search(*query);
    cout << format("{} -> {} document(s) ({})\n", query->toString(), results.size(),
                   timer.elapsed());

    for (size_t i = 0; i < results.size() && i < kMaxResultsShown; ++i) {
        cout << format("  [{}] {}\n", results[i], snippet(engine_.document(results[i])));
    }
    if (results.size() > kMaxResultsShown) {
        cout << format("  ... and {} more\n", results.size() - kMaxResultsShown);
    }
    if (results.empty()) suggestCorrections(*query);
}

void Shell::suggestCorrections(const QueryNode& query) const {
    vector<string> terms;
    query.collectTerms(terms);
    for (const string& term : terms) {
        if (engine_.hasTerm(term)) continue;
        const vector<Correction> corrections = engine_.didYouMean(term, kMaxTypos, 1);
        if (!corrections.empty()) {
            cout << format("  '{}' is not indexed. Did you mean '{}'?\n", term,
                           corrections.front().word);
        }
    }
}

void Shell::complete(string_view prefix) const {
    const Stopwatch timer;
    const vector<Suggestion> suggestions = engine_.autocomplete(prefix, kSuggestionCount);
    cout << format("{} completion(s) for '{}' ({})\n", suggestions.size(), prefix, timer.elapsed());
    for (const Suggestion& suggestion : suggestions) {
        cout << format("  {:<24} freq {}\n", suggestion.word, suggestion.weight);
    }
}

void Shell::fix(string_view word) const {
    const Stopwatch timer;
    const vector<Correction> corrections = engine_.didYouMean(word, kMaxTypos, kSuggestionCount);
    cout << format("{} suggestion(s) for '{}' ({})\n", corrections.size(), word, timer.elapsed());
    for (const Correction& correction : corrections) {
        cout << format("  {:<24} {} edit(s), freq {}\n", correction.word, correction.distance,
                       correction.weight);
    }
}

void Shell::printStats() const {
    cout << format("{} documents, {} distinct terms\n", engine_.documentCount(),
                   engine_.vocabularySize());
}

}  // namespace seek::cli
