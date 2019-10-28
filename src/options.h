#pragma once

struct Options {
    std::string separator = ".";
    std::string wildcard_one = "*";
    std::string wildcard_some = "#";
    bool cache_adds = false;
};
