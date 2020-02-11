#pragma once

#include <string>
#include <variant>

struct Options {
    std::string separator = ".";
    std::string wildcard_one = "*";
    std::string wildcard_some = "#";
    bool match_empty_levels = false;
    bool cache_adds = false;
    std::size_t max_words = 100;
    std::size_t max_wildcard_somes = 3;
};

template<typename State>
struct OptionsOrState {
    bool is_options;
    std::variant<Options, State*> data;
};
