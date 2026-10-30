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

# Core library.
add_library(seek
    src/autocomplete.cpp
    src/fuzzy.cpp
    src/inverted_index.cpp
    src/posting_list.cpp
    src/query_executor.cpp
    src/query_parser.cpp
    src/search_engine.cpp
    src/top_k_cache.cpp
    src/tokenizer.cpp
    src/trie.cpp
)
add_library(seek::seek ALIAS seek)
target_include_directories(seek PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(seek PRIVATE seek_options)

# Interactive shell.
add_executable(seek_cli apps/seek_cli.cpp)
set_target_properties(seek_cli PROPERTIES OUTPUT_NAME seek)
target_link_libraries(seek_cli PRIVATE seek::seek seek_options)

if(SEEK_BUILD_BENCHMARKS)
    add_executable(seek_bench bench/seek_bench.cpp)
    target_link_libraries(seek_bench PRIVATE seek::seek seek_options)
endif()

if(SEEK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
