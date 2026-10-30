#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "seek/autocomplete.hpp"
#include "seek/fuzzy.hpp"
#include "seek/inverted_index.hpp"
#include "seek/query.hpp"
#include "seek/top_k_cache.hpp"
#include "seek/trie.hpp"
#include "seek/types.hpp"

namespace seek {

struct SearchEngineOptions {
    // Prefixes up to this length get precomputed top-k suggestions (see TopKCache).
    std::size_t cachedPrefixLength = 3;
    // Largest k answered from that cache; bigger requests fall back to the trie walk.
    std::size_t cachedSuggestions = 10;
};

// Facade over the subsystems. Indexing a document tokenizes it once and feeds
//   - the inverted index (term -> documents), used by boolean search,
//   - the vocabulary trie (term -> corpus frequency), used by autocomplete and "did you mean",
//   - the top-k cache, which answers autocomplete for short prefixes without a trie walk.
//
// Thread safety: const methods may run concurrently with each other, but not with
// addDocument(); callers that interleave writes need external synchronization.
class SearchEngine {
public:
    explicit SearchEngine(SearchEngineOptions options = {});

    // Indexes `text` and returns its id. Ids are dense: 0, 1, 2, ...
    DocId addDocument(std::string text);

    // Documents matching a boolean query such as "distributed AND (storage OR database)",
    // in ascending id order. Throws QueryError on malformed queries.
    [[nodiscard]] std::vector<DocId> search(std::string_view query) const;
    [[nodiscard]] std::vector<DocId> search(const Query& query) const;

    // Up to `k` indexed terms starting with `prefix` (case-insensitive), most frequent first.
    [[nodiscard]] std::vector<Suggestion> autocomplete(std::string_view prefix,
                                                       std::size_t k = 5) const;

    // Up to `limit` indexed terms within `maxDistance` edits of `word`: closest first,
    // then most frequent.
    [[nodiscard]] std::vector<FuzzyMatch> didYouMean(std::string_view word,
                                                     std::size_t maxDistance = 2,
                                                     std::size_t limit = 5) const;

    // Whether `term` (case-insensitive) occurs in any indexed document.
    [[nodiscard]] bool hasTerm(std::string_view term) const;

    // Original text of a document. Throws std::out_of_range for an unknown id.
    [[nodiscard]] const std::string& document(DocId id) const;

    [[nodiscard]] std::size_t documentCount() const noexcept { return documents_.size(); }
    [[nodiscard]] std::size_t vocabularySize() const noexcept { return vocabulary_.wordCount(); }

private:
    std::vector<std::string> documents_;  // indexed by DocId
    InvertedIndex index_;
    Trie vocabulary_;
    TopKCache hotPrefixes_;
};

}  // namespace seek
