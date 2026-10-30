// seek: interactive search shell over plain-text files (one document per non-empty line).
//
//   ./build/seek data/sample_corpus.txt
//   seek> distributed AND (storage OR database)
//   seek> :complete dat
//   seek> :fix distrbuted

#include <unistd.h>

#include <cstdio>
#include <format>
#include <iostream>
#include <string>

#include "corpus_loader.hpp"
#include "seek/engine/search_engine.hpp"
#include "shell.hpp"
#include "stopwatch.hpp"

using namespace std;
using namespace seek;
using namespace seek::cli;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "usage: seek <corpus.txt>...   (one document per non-empty line)\n";
        return 2;
    }

    SearchEngine engine;
    const Stopwatch timer;
    for (int i = 1; i < argc; ++i) {
        if (!loadCorpus(argv[i], engine)) return 1;
    }
    cout << format("Indexed {} documents ({} distinct terms) in {}. Type :help.\n",
                   engine.documentCount(), engine.vocabularySize(), timer.elapsed());

    // When input is piped in rather than typed, echo each line so the output reads as a transcript.
    const bool interactive = isatty(fileno(stdin)) != 0;
    Shell shell(engine);
    string line;
    while (true) {
        if (interactive) cout << "seek> " << flush;
        if (!getline(cin, line)) break;
        if (!interactive) cout << "seek> " << line << '\n';
        if (!shell.execute(line)) break;
    }
    return 0;
}
