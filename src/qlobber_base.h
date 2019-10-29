#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <functional>
#include "options.h"

template<typename Value,
         typename ValueStorage,
         typename Remove,
         typename MatchResult,
         typename Context,
         typename Test>
class QlobberBase {
public:
    QlobberBase() {}
        // TODO: implement Qlobber.native and QlobberDup.native
        //         to check algo is correct by testing and benchmarking them
        // TODO: async
        // TODO: worker threads

    QlobberBase(const Options& options) : options(options) {}

    virtual void add(const std::string& topic, const Value& val) {
        if (options.cache_adds) {
            const auto it = shortcuts.find(topic);
            if (it != shortcuts.end()) {
                return add_value(it->second, val);
            }
        }
        auto& storage = add(val, 0, split(topic), trie);
        if (options.cache_adds) {
            shortcuts.emplace(topic, storage);
        }
    }

    virtual void remove(const std::string& topic,
                        const std::optional<const Remove>& val) {
        if (remove(val, 0, split(topic), trie) && options.cache_adds) {
            shortcuts.erase(topic);
        }
    }

    virtual void match(MatchResult& r, const std::string& topic, Context& ctx) {
        match(r, 0, split(topic), trie, ctx);
    }

    virtual bool test(const std::string& topic, const Test& val) {
        return test(val, 0, split(topic), trie);
    }

    virtual void clear() {
        shortcuts.clear();
        std::get<0>(trie.v)->clear();
    }

    virtual bool test_values(const ValueStorage& vals,
                             const Test& val) = 0;

protected:
    Options options;
    std::unordered_map<std::string, std::reference_wrapper<ValueStorage>> shortcuts;

private:
    struct Trie {
        typedef std::unordered_map<std::string, Trie> map_type;
        typedef std::unique_ptr<map_type> map_ptr;
        Trie() : v(std::in_place_index<0>, std::make_unique<map_type>()) {}
        Trie(const Value& val) : v(std::in_place_index<1>, val) {}
        std::variant<map_ptr, ValueStorage> v;
    } trie;

    ValueStorage& add(const Value& val,
                      const std::size_t i,
                      const std::vector<std::string>& words,
                      const Trie& sub_trie) {
        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(options.separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                auto& storage = std::get<1>(it->second.v);
                add_value(storage, val);
                return storage;
            }

            std::get<0>(sub_trie.v)->emplace(options.separator, val);
            initial_value_inserted(val);
            return std::get<1>((*std::get<0>(sub_trie.v))[options.separator].v);
        }

        return add(val, i + 1, words, (*std::get<0>(sub_trie.v))[words[i]]);
    }

    bool remove(const std::optional<const Remove>& val,
                const std::size_t i,
                const std::vector<std::string>& words,
                const Trie& sub_trie) {
        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(options.separator);

            if ((it != std::get<0>(sub_trie.v)->end()) &&
                remove_value(std::get<1>(it->second.v), val)) {
                std::get<0>(sub_trie.v)->erase(it);
                return true;
            }

            return false;
        }

        const auto& word = words[i];
        const auto it = std::get<0>(sub_trie.v)->find(word);

        if (it == std::get<0>(sub_trie.v)->end()) {
            return false;
        }

        const auto r = remove(val, i + 1, words, it->second);

        if (std::get<0>(it->second.v)->empty()) {
            std::get<0>(sub_trie.v)->erase(it);
        }

        return r;
    }

    void match_some(MatchResult& r,
                    const std::size_t i,
                    const std::vector<std::string>& words,
                    const Trie& st,
                    Context& ctx) {
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != options.separator) {
                for (std::size_t j = i; j < words.size(); ++j) {
                    match(r, j, words, st, ctx);
                }
                break;
            }
        }
    }

    void match(MatchResult& r,
               const std::size_t i,
               const std::vector<std::string>& words,
               const Trie& sub_trie,
               Context& ctx) {
        {
            const auto it = std::get<0>(sub_trie.v)->find(options.wildcard_some);

            if (it != std::get<0>(sub_trie.v)->end()) {
                // in the common case there will be no more levels...
                match_some(r, i, words, it->second, ctx);
                // and we'll end up matching the rest of the words:
                match(r, words.size(), words, it->second, ctx);
            }
        }

        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(options.separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                add_values(r, std::get<1>(it->second.v), ctx);
            }
        } else {
            const auto& word = words[i];

            if ((word != options.wildcard_one) &&
                (word != options.wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match(r, i + 1, words, it->second, ctx);
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(options.wildcard_one);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match(r, i + 1, words, it->second, ctx);
                }
            }
        }
    }

    bool test_some(const Test& v,
                   const std::size_t i,
                   const std::vector<std::string>& words,
                   const Trie& st) {
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != options.separator) {
                for (std::size_t j = i; j < words.size(); ++j) {
                    if (test(v, j, words, st)) {
                        return true;
                    }
                }
                break;
            }
        }

        return false;
    }

    bool test(const Test& v,
              const std::size_t i,
              const std::vector<std::string>& words,
              const Trie& sub_trie) {
        {
            const auto it = std::get<0>(sub_trie.v)->find(options.wildcard_some);

            if (it != std::get<0>(sub_trie.v)->end()) {
                    // in the common case there will be no more levels...
                if (test_some(v, i, words, it->second) ||
                    // and we'll end up matching the rest of the words:
                    test(v, words.size(), words, it->second)) {
                    return true;
                }
            }
        }

        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(options.separator);

            if ((it != std::get<0>(sub_trie.v)->end()) &&
                test_values(std::get<1>(it->second.v), v)) {
                return true;
            }
        } else {
            const auto& word = words[i];

            if ((word != options.wildcard_one) &&
                (word != options.wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if ((it != std::get<0>(sub_trie.v)->end()) &&
                    test(v, i + 1, words, it->second)) {
                    return true;
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(options.wildcard_one);

                if ((it != std::get<0>(sub_trie.v)->end()) &&
                    test(v, i + 1, words, it->second)) {
                    return true;
                }
            }
        }

        return false;
    }

    std::vector<std::string> split(const std::string& topic) {
        std::vector<std::string> words;
        std::size_t last = 0;
        while (true) {
            std::size_t next = topic.find(options.separator, last);
            if (next == std::string::npos) {
                break;
            }
            words.push_back(std::move(topic.substr(last, next - last)));
            last = next + 1;
        }
        words.push_back(std::move(topic.substr(last)));
        return words;
    }

    virtual void initial_value_inserted(const Value& val) {}
    virtual void add_value(ValueStorage& vals, const Value& val) = 0;
    virtual bool remove_value(ValueStorage& vals,
                              const std::optional<const Remove>& val) = 0;
    virtual void add_values(MatchResult& r,
                            const ValueStorage& vals,
                            Context& ctx) = 0;
};
