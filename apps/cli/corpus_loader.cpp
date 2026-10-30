#include "corpus_loader.hpp"

#include <fstream>
#include <iostream>

using namespace std;

namespace seek::cli {

bool loadCorpus(const string& path, SearchEngine& engine) {
    ifstream file(path);
    if (!file) {
        cerr << "seek: cannot open '" << path << "'\n";
        return false;
    }
    string line;
    while (getline(file, line)) {
        if (line.find_first_not_of(" \t\r") != string::npos) engine.addDocument(line);
    }
    return true;
}

}  // namespace seek::cli
