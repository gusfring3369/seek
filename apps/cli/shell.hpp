#pragma once

#include <string>
#include <string_view>

#include "seek/engine/search_engine.hpp"

namespace seek::cli {

// The interactive prompt: reads one line at a time and runs it.
//
//   <query>             boolean search
//   :complete <prefix>  autocomplete              (alias :c)
//   :fix <word>         "did you mean"            (alias :f)
//   :stats, :help, :quit
class Shell {
public:
    explicit Shell(const SearchEngine& engine) : engine_(engine) {}

    // Runs one input line. Returns false when the user asked to quit.
    bool execute(std::string_view line);

    static void printHelp();

private:
    void search(std::string_view queryText) const;
    void suggestCorrections(const QueryNode& query) const;
    void complete(std::string_view prefix) const;
    void fix(std::string_view word) const;
    void printStats() const;

    const SearchEngine& engine_;
};

}  // namespace seek::cli
