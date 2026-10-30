find_package(GTest 1.11 QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(seek_tests
    autocomplete_test.cpp
    fuzzy_test.cpp
    inverted_index_test.cpp
    posting_list_test.cpp
    query_test.cpp
    search_engine_test.cpp
    tokenizer_test.cpp
    top_k_cache_test.cpp
    trie_test.cpp
)
target_link_libraries(seek_tests PRIVATE seek::seek seek_options GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(seek_tests)

# End-to-end check: drive the CLI with a scripted session over the sample corpus.
add_test(NAME cli_smoke
    COMMAND sh -c "printf 'distributed AND (storage OR database)\\n:complete dat\\n:fix distrbuted\\n:quit\\n' | $<TARGET_FILE:seek_cli> ${PROJECT_SOURCE_DIR}/data/sample_corpus.txt")
set_tests_properties(cli_smoke PROPERTIES
    PASS_REGULAR_EXPRESSION "Did you mean|distributed[ ]+1 edit")
