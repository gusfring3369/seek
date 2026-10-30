// seek: interactive search shell over plain-text corpora (one document per non-empty line).
//
//   seek data/sample_corpus.txt
//   seek> distributed AND (storage OR database)
//   seek> :complete dat
//   seek> :fix distrbuted

#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "seek/query.hpp"
#include "seek/search_engine.hpp"

namespace {

constexpr std::size_t kMaxHitsShown = 10;
constexpr std::size_t kMaxSnippetLength = 96;
constexpr std::size_t kSuggestions = 5;

constexpr std::string_view kHelp = R"(Commands:
  <query>             boolean search, e.g.  distributed AND (storage OR database)
                      AND binds tighter than OR; adjacent words are ANDed
  :complete <prefix>  top-5 completions by corpus frequency   (alias :c)
  :fix <word>         "did you mean" within 2 edits          (alias :f)
  :stats              corpus statistics
  :help               this message
  :quit               exit                                    (or Ctrl-D)
)";

class Stopwatch {
public:
    [[nodiscard]] std::string elapsed() const {
        const std::chrono::duration<double, std::micro> us =
            std::chrono::steady_clock::now() - start_;
        return us.count() < 1000 ? std::format("{:.1f} µs", us.count())
                                 : std::format("{:.2f} ms", us.count() / 1e3);
    }

private:
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
};

std::string_view trim(std::string_view s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::string snippet(std::string_view text) {
    if (text.size() <= kMaxSnippetLength) return std::string(text);
    return std::string(text.substr(0, kMaxSnippetLength - 3)) + "...";
}

void collectTerms(const seek::Query& query, std::vector<std::string>& out) {
    if (query.kind == seek::Query::Kind::Term) {
        out.push_back(query.term);
        return;
    }
    for (const seek::Query& child : query.children) collectTerms(child, out);
}

bool loadCorpus(const std::string& path, seek::SearchEngine& engine) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << std::format("seek: cannot open '{}'\n", path);
        return false;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (!trim(line).empty()) engine.addDocument(std::string(trim(line)));
    }
    return true;
}

class Shell {
public:
    explicit Shell(const seek::SearchEngine& engine) : engine_(engine) {}

    // Returns false when the user asked to quit.
    bool handle(std::string_view line) {
        line = trim(line);
        if (line.empty()) return true;
        if (line.front() != ':') {
            search(line);
            return true;
        }

        const auto space = line.find(' ');
        const std::string_view command = line.substr(0, space);
        const std::string_view argument =
            space == std::string_view::npos ? std::string_view{} : trim(line.substr(space));

        if (command == ":quit" || command == ":q") return false;
        if (command == ":help" || command == ":h") {
            std::cout << kHelp;
        } else if (command == ":stats") {
            stats();
        } else if ((command == ":complete" || command == ":c") && !argument.empty()) {
            complete(argument);
        } else if ((command == ":fix" || command == ":f") && !argument.empty()) {
            fix(argument);
        } else {
            std::cout << "Unknown command or missing argument. Type :help.\n";
        }
        return true;
    }

private:
    void search(std::string_view text) const {
        const Stopwatch timer;
        seek::Query query;
        try {
            query = seek::parseQuery(text);
        } catch (const seek::QueryError& e) {
            std::cout << "Query error: " << e.what() << '\n';
            return;
        }
        const std::vector<seek::DocId> hits = engine_.search(query);
        std::cout << std::format("{} -> {} document(s) ({})\n", seek::toString(query), hits.size(),
                                 timer.elapsed());

        for (std::size_t i = 0; i < hits.size() && i < kMaxHitsShown; ++i) {
            std::cout << std::format("  [{}] {}\n", hits[i], snippet(engine_.document(hits[i])));
        }
        if (hits.size() > kMaxHitsShown) {
            std::cout << std::format("  ... and {} more\n", hits.size() - kMaxHitsShown);
        }
        if (hits.empty()) suggestCorrections(query);
    }

    void suggestCorrections(const seek::Query& query) const {
        std::vector<std::string> terms;
        collectTerms(query, terms);
        for (const std::string& term : terms) {
            if (engine_.hasTerm(term)) continue;
            const auto matches = engine_.didYouMean(term, 2, 1);
            if (!matches.empty()) {
                std::cout << std::format("  '{}' is not indexed. Did you mean '{}'?\n", term,
                                         matches.front().word);
            }
        }
    }

    void complete(std::string_view prefix) const {
        const Stopwatch timer;
        const auto suggestions = engine_.autocomplete(prefix, kSuggestions);
        std::cout << std::format("{} completion(s) for '{}' ({})\n", suggestions.size(), prefix,
                                 timer.elapsed());
        for (const auto& s : suggestions) {
            std::cout << std::format("  {:<24} freq {}\n", s.word, s.weight);
        }
    }

    void fix(std::string_view word) const {
        const Stopwatch timer;
        const auto matches = engine_.didYouMean(word, 2, kSuggestions);
        std::cout << std::format("{} suggestion(s) for '{}' ({})\n", matches.size(), word,
                                 timer.elapsed());
        for (const auto& m : matches) {
            std::cout << std::format("  {:<24} {} edit(s), freq {}\n", m.word, m.distance,
                                     m.weight);
        }
    }

    void stats() const {
        std::cout << std::format("{} documents, {} distinct terms\n", engine_.documentCount(),
                                 engine_.vocabularySize());
    }

    const seek::SearchEngine& engine_;
};

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: seek <corpus.txt>...   (one document per non-empty line)\n";
        return 2;
    }

    seek::SearchEngine engine;
    const Stopwatch timer;
    for (int i = 1; i < argc; ++i) {
        if (!loadCorpus(argv[i], engine)) return 1;
    }
    std::cout << std::format("Indexed {} documents ({} distinct terms) in {}. Type :help.\n",
                             engine.documentCount(), engine.vocabularySize(), timer.elapsed());

    const bool interactive = isatty(fileno(stdin)) != 0;
    Shell shell(engine);
    std::string line;
    while (true) {
        if (interactive) std::cout << "seek> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (!interactive) std::cout << "seek> " << line << '\n';  // echo for transcripts
        if (!shell.handle(line)) break;
    }
    return 0;
}
