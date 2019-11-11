#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <functional>
#include <shared_mutex>
#include <boost/coroutine2/all.hpp>
#include "options.h"

template <typename Value>
using VisitData = std::variant<std::string, Value>;

template<typename Value>
struct Visit {
    enum {
        start_entries,
        entry,
        end_entries,
        start_values,
        value,
        end_values
    } type;
    std::optional<VisitData<Value>> v;
};

template<typename Value,
         typename ValueStorage,
         typename Remove,
         typename MatchResult,
         typename Context,
         typename Test,
         typename IterValue = Value>
class QlobberBase {
public:
    QlobberBase() {}
        // TODO: can we fold other separate functions into classes?
        // TODO: async
        //   either pass in fn to match or make a new class which can
        //   access same store and has non-JS types
        //   we need this for worker threads anyway
        //   OR use method template
        // TODO: worker threads

    QlobberBase(const Options& options) : options(options) {}

    typedef typename boost::coroutines2::coroutine<Visit<Value>> coro_visit_t;

    typename coro_visit_t::push_type restore(bool cache_adds = false) {
        return typename coro_visit_t::push_type(
            std::bind(&QlobberBase::inject, this, std::placeholders::_1, cache_adds));
    }

    typedef typename boost::coroutines2::coroutine<IterValue> coro_iter_t;

    typename coro_iter_t::pull_type match_iter(const std::string& topic,
                                               Context& ctx) {
        // "The arguments to bind are copied or moved, and are never passed by
        // reference unless wrapped in std::ref or std::cref."
        // https://en.cppreference.com/w/cpp/utility/functional/bind
        // So it doesn't matter if topic and ctx are destroyed later.
        return typename coro_iter_t::pull_type(
            std::bind(&QlobberBase::generate_values, this, std::placeholders::_1, topic, ctx));
    }

protected:
    void add(const std::string& topic, const Value& val) {
        std::unique_lock lock(mutex);
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

    void remove(const std::string& topic,
                const std::optional<const Remove>& val) {
        std::unique_lock lock(mutex);
        if (remove(val, 0, split(topic), trie) && options.cache_adds) {
            shortcuts.erase(topic);
        }
    }

    void match(MatchResult& r, const std::string& topic, Context& ctx) {
        std::shared_lock lock(mutex);
        match(r, 0, split(topic), trie, ctx);
    }

    bool test(const std::string& topic, const Test& val) {
        std::shared_lock lock(mutex);
        return test(val, 0, split(topic), trie);
    }

    virtual void clear() {
        std::unique_lock lock(mutex);
        shortcuts.clear();
        std::get<0>(trie.v)->clear();
    }

    typename coro_visit_t::pull_type visit() {
        return typename coro_visit_t::pull_type(
            std::bind(&QlobberBase::generate_visits, this, std::placeholders::_1));
    }

    Options options;
    std::unordered_map<std::string, std::reference_wrapper<ValueStorage>> shortcuts;
    std::shared_mutex mutex;

    virtual void add_values(MatchResult& r,
                            const ValueStorage& vals,
                            Context& ctx) = 0;

private:
    virtual void initial_value_inserted(const Value& val) {}
    virtual void add_value(ValueStorage& vals, const Value& val) = 0;
    virtual bool remove_value(ValueStorage& vals,
                              const std::optional<const Remove>& val) = 0;
    virtual bool test_values(const ValueStorage& vals,
                             const Test& val) = 0;
    virtual void iter_values(typename coro_iter_t::push_type& sink,
                             const ValueStorage& vals,
                             Context& ctx) = 0;
    virtual void visit_values(typename coro_visit_t::push_type& sink,
                              const ValueStorage& storage) = 0;

    struct Trie {
        typedef std::unordered_map<std::string, Trie> map_type;
        typedef std::unique_ptr<map_type> map_ptr;
        Trie() : v(std::in_place_index<0>, std::make_unique<map_type>()) {}
        Trie(const Value& val) : v(std::in_place_index<1>, val) {}
        Trie(std::shared_ptr<Trie>& t) : v(std::move(t->v)) {}
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

    void match_some_iter(typename coro_iter_t::push_type& sink,
                         const std::size_t i,
                         const std::vector<std::string>& words,
                         const Trie& st,
                         Context& ctx) {
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != options.separator) {
                for (std::size_t j = i; j < words.size(); ++j) {
                    match_iter(sink, j, words, st, ctx);
                }
                break;
            }
        }
    }

    void match_iter(typename coro_iter_t::push_type& sink,
                    const std::size_t i,
                    const std::vector<std::string>& words,
                    const Trie& sub_trie,
                    Context& ctx) {
        {
            const auto it = std::get<0>(sub_trie.v)->find(options.wildcard_some);

            if (it != std::get<0>(sub_trie.v)->end()) {
                // in the common case there will be no more levels...
                match_some_iter(sink, i, words, it->second, ctx);
                // and we'll end up matching the rest of the words:
                match_iter(sink, words.size(), words, it->second, ctx);
            }
        }

        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(options.separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                iter_values(sink, std::get<1>(it->second.v), ctx);
            }
        } else {
            const auto& word = words[i];

            if ((word != options.wildcard_one) &&
                (word != options.wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match_iter(sink, i + 1, words, it->second, ctx);
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(options.wildcard_one);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match_iter(sink, i + 1, words, it->second, ctx);
                }
            }
        }
    }

    void generate_values(typename coro_iter_t::push_type& sink,
                         const std::string& topic,
                         Context& ctx) {
        std::shared_lock lock(mutex);
        match_iter(sink, 0, split(topic), trie, ctx);
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

    void generate_visits(typename coro_visit_t::push_type& sink) {
        struct Saved {
            typename Trie::map_type::const_iterator it, end;
            std::size_t i;
        } cur = {
            std::get<0>(trie.v)->cbegin(),
            std::get<0>(trie.v)->cend(),
            0
        };
        std::vector<Saved> saved;

        while (true) {
            if (cur.i == 0) {
                sink({ Visit<Value>::start_entries, std::nullopt });
            }

            if (cur.it == cur.end) {
                sink({ Visit<Value>::end_entries, std::nullopt });

                if (saved.empty()) {
                    return;
                }

                cur = saved.back();
                saved.pop_back();
                continue;
            }

            sink({
                Visit<Value>::entry,
                VisitData<Value> {
                    std::variant<std::string, Value>(
                        std::in_place_index<0>, cur.it->first)
                }
            });

            ++cur.i;

            if (cur.it->first == options.separator) {
                sink({ Visit<Value>::start_values, std::nullopt });
                visit_values(sink, std::get<1>(cur.it->second.v));
                sink({ Visit<Value>::end_values, std::nullopt });
                ++cur.it;
                continue;
            }

            saved.push_back(cur);
            cur.end = std::get<0>(cur.it->second.v)->cend();
            cur.it = std::get<0>(cur.it->second.v)->cbegin();
            cur.i = 0;
            ++saved.back().it;
        }

    }

    void inject(typename coro_visit_t::pull_type& source, bool cache_adds) {
        struct Saved {
            Saved(std::shared_ptr<Trie> entry,
                  const std::string& key,
                  const std::string& path) :
                entry(entry), key(key), path(path) {}
            std::shared_ptr<Trie> entry;
            std::string key, path;
        };
        std::vector<Saved> saved;
        std::shared_ptr<Trie> entry(&trie, [](Trie*) {});
        std::string path;

        for (const auto& v : source) {
            switch (v.type) {
                case Visit<Value>::entry: {
                    if (!entry) {
                        entry = std::make_shared<Trie>();
                    }
                    const auto& key = std::get<0>(*v.v);
                    saved.emplace_back(entry, key, path);
                    const auto it = std::get<0>(entry->v)->find(key);
                    if (it == std::get<0>(entry->v)->end()) {
                        entry.reset();
                    } else {
                        entry.reset(&it->second, [](Trie*) {});
                    }
                    if (cache_adds) {
                        if (!path.empty()) {
                            path += options.separator;
                        }
                        path += key;
                    }
                    break;
                }

                case Visit<Value>::value: {
                    const auto& val = std::get<1>(*v.v);
                    if (entry) {
                        add_value(std::get<1>(entry->v), val);
                    } else {
                        entry = std::make_unique<Trie>(val);
                        initial_value_inserted(val);
                    }
                    break;
                }

                case Visit<Value>::end_entries:
                    if (entry && std::get<0>(entry->v)->empty()) {
                        entry.reset();
                    }
                    // falls through

                case Visit<Value>::end_values:
                    if (saved.empty()) {
                        entry.reset();
                        path.clear();
                    } else {
                        if (entry) {
                            const auto it = std::get<0>(saved.back().entry->v)->emplace(
                                saved.back().key, entry);
                            if (cache_adds &&
                                options.cache_adds &&
                                (v.type == Visit<Value>::end_values)) {
                                shortcuts.emplace(saved.back().path,
                                                  std::get<1>(it.first->second.v));
                            }
                        }
                        entry = saved.back().entry;
                        path = saved.back().path;
                        saved.pop_back();
                    }
                    break;

                case Visit<Value>::start_entries:
                case Visit<Value>::start_values:
                    break;
            }
        }
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
};
