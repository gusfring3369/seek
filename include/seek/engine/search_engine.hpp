#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "seek/autocomplete/cached_autocompleter.hpp"
#include "seek/autocomplete/trie_autocompleter.hpp"
#include "seek/engine/document_store.hpp"
#include "seek/index/adaptive_intersection.hpp"
#include "seek/index/inverted_index.hpp"
#include "seek/query/query_parser.hpp"
#include "seek/spellcheck/spell_checker.hpp"
#include "seek/text/tokenizer.hpp"
#include "seek/trie/trie.hpp"

namespace seek {

struct SearchEngineOptions {
    // Prefixes up to this many characters are answered from the autocomplete cache.
    std::size_t cachedPrefixLength = 3;
    // The largest k the cache can answer; bigger requests walk the trie.
    std::size_t cachedSuggestions = 10;
};

// The public entry point (Facade pattern): one object that hides how the parts fit together.
//
//   addDocument(text)
//     text -> Tokenizer -> terms ─┬─> InvertedIndex       (term -> documents)  used by search()
//                                 ├─> Trie vocabulary     (term -> frequency)  used by
//                                 autocomplete() │                                              and
//                                 didYouMean() └─> CachedAutocompleter (short prefix -> top
//                                 suggestions)
//
//   search(query)       QueryParser -> QueryNode tree -> evaluate against the InvertedIndex
//   autocomplete(p, k)  CachedAutocompleter, which falls back to TrieAutocompleter
//   didYouMean(word)    SpellChecker over the vocabulary trie
//
// Thread safety: const methods may run at the same time as each other, but not at the same time
// as addDocument().
class SearchEngine {
public:
    explicit SearchEngine(SearchEngineOptions options = {});

    // Components hold references to each other, so the engine cannot be copied or moved.
    SearchEngine(const SearchEngine&) = delete;
    SearchEngine& operator=(const SearchEngine&) = delete;

    // Indexes `text` and returns its id.
    DocId addDocument(std::string text);

    // Ids of the documents matching a boolean query such as
    // "distributed AND (storage OR database)", in increasing order.
    // Throws QueryError if the query is malformed.
    [[nodiscard]] std::vector<DocId> search(std::string_view query) const;
    [[nodiscard]] std::vector<DocId> search(const QueryNode& query) const;

    // Parses a query without running it. Throws QueryError.
    [[nodiscard]] std::unique_ptr<QueryNode> parse(std::string_view query) const;

    // Up to `k` indexed words that start with `prefix` (any case), most frequent first.
    [[nodiscard]] std::vector<Suggestion> autocomplete(std::string_view prefix,
                                                       std::size_t k = 5) const;

    // Up to `limit` indexed words within `maxDistance` edits of `word` (any case):
    // closest first, then most frequent.
    [[nodiscard]] std::vector<Correction> didYouMean(std::string_view word,
                                                     std::size_t maxDistance = 2,
                                                     std::size_t limit = 5) const;

    // Whether `term` (any case) appears in any indexed document.
    [[nodiscard]] bool hasTerm(std::string_view term) const;

    // The original text of a document. Throws std::out_of_range for an unknown id.
    [[nodiscard]] const std::string& document(DocId id) const { return documents_.get(id); }

    [[nodiscard]] std::size_t documentCount() const noexcept { return documents_.size(); }
    [[nodiscard]] std::size_t vocabularySize() const noexcept { return vocabulary_.wordCount(); }

private:
    // Declaration order matters: members are built top to bottom, and the later ones keep
    // references to the earlier ones.
    Tokenizer tokenizer_;
    DocumentStore documents_;
    InvertedIndex index_;
    Trie vocabulary_;
    AdaptiveIntersection intersection_;
    QueryParser parser_;                   // uses tokenizer_
    TrieAutocompleter trieAutocompleter_;  // reads vocabulary_
    CachedAutocompleter autocompleter_;    // falls back to trieAutocompleter_
    SpellChecker spellChecker_;            // reads vocabulary_
};

}  // namespace seek
