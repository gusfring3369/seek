cmake_minimum_required(VERSION 3.20)
project(seek VERSION 1.0.0 DESCRIPTION "In-memory full-text search engine with fuzzy autocomplete"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(SEEK_BUILD_TESTS "Build unit tests" ON)
option(SEEK_BUILD_BENCHMARKS "Build benchmarks" ON)
option(SEEK_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(SEEK_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# Shared compile settings for every target in the project.
add_library(seek_options INTERFACE)
if(MSVC)
    target_compile_options(seek_options INTERFACE /W4 $<$<BOOL:${SEEK_WARNINGS_AS_ERRORS}>:/WX>)
else()
    target_compile_options(seek_options INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
        -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
        $<$<BOOL:${SEEK_WARNINGS_AS_ERRORS}>:-Werror>)
    if(SEEK_SANITIZE)
        target_compile_options(seek_options INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(seek_options INTERFACE -fsanitize=address,undefined)
    endif()
endif()

# Core library, one folder per subsystem.
add_library(seek
    src/text/tokenizer.cpp
    src/trie/trie.cpp
    src/autocomplete/top_k_heap.cpp
    src/autocomplete/trie_autocompleter.cpp
    src/autocomplete/cached_autocompleter.cpp
    src/spellcheck/edit_distance.cpp
    src/spellcheck/spell_checker.cpp
    src/index/inverted_index.cpp
    src/index/two_pointer_intersection.cpp
    src/index/galloping_intersection.cpp
    src/index/adaptive_intersection.cpp
    src/index/posting_list_union.cpp
    src/query/query_token.cpp
    src/query/query_lexer.cpp
    src/query/term_node.cpp
    src/query/composite_node.cpp
    src/query/and_node.cpp
    src/query/or_node.cpp
    src/query/query_parser.cpp
    src/engine/document_store.cpp
    src/engine/search_engine.cpp
)
add_library(seek::seek ALIAS seek)
target_include_directories(seek PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(seek PRIVATE seek_options)

# Interactive shell (the executable is called `seek`).
add_executable(seek_cli
    apps/cli/main.cpp
    apps/cli/shell.cpp
    apps/cli/corpus_loader.cpp
)
set_target_properties(seek_cli PROPERTIES OUTPUT_NAME seek)
target_link_libraries(seek_cli PRIVATE seek::seek seek_options)

if(SEEK_BUILD_BENCHMARKS)
    add_executable(seek_bench
        bench/main.cpp
        bench/bench_utils.cpp
        bench/autocomplete_bench.cpp
        bench/spellcheck_bench.cpp
        bench/intersection_bench.cpp
    )
    target_link_libraries(seek_bench PRIVATE seek::seek seek_options)
endif()

if(SEEK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
