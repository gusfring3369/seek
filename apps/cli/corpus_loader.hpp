#pragma once

#include <string>

#include "seek/engine/search_engine.hpp"

namespace seek::cli {

// Reads a plain-text file and indexes every non-empty line as one document.
// Returns false (after printing an error) if the file cannot be opened.
bool loadCorpus(const std::string& path, SearchEngine& engine);

}  // namespace seek::cli
