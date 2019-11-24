#pragma once

struct Options {
    std::string separator = ".";
    std::string wildcard_one = "*";
    std::string wildcard_some = "#";
    bool cache_adds = false;
    std::size_t max_words = 100;
    std::size_t max_wildcard_somes = 5;
};
