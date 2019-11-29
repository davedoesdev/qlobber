#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <functional>
#include <atomic>
#include <boost/coroutine2/all.hpp>
#include "options.h"
#include "rwlock.h"

template <typename Value>
using VisitData = std::variant<std::string, Value>;

template<typename Value>
struct Visit {
    enum Type {
        start_entries,
        entry,
        end_entries,
        start_values,
        value,
        end_values
    };
    Visit(Type type, std::optional<VisitData<Value>>&& v) :
        type(type), v(v) {};
    Type type;
    std::optional<VisitData<Value>> v;
};

struct EmptyStateBase {};

template<typename Value,
         typename ValueStorage,
         typename RemoveValue,
         typename MatchResult,
         typename Context,
         typename TestValue,
         typename IterValue = Value,
         typename StateBase = EmptyStateBase>
class QlobberBase {
public:
    struct Trie {
        typedef std::unordered_map<std::string, Trie> map_type;
        typedef std::unique_ptr<map_type> map_ptr;
        Trie() : v(std::in_place_index<0>, std::make_unique<map_type>()) {}
        Trie(const Value& val) : v(std::in_place_index<1>, val) {}
        Trie(std::shared_ptr<Trie>& t) : v(std::move(t->v)) {}
        std::variant<map_ptr, ValueStorage> v;
    };

    struct State : public StateBase {
        std::atomic<uint32_t> ref_count;
        RWLock rwlock;
        Options options;
        Trie trie;
        std::unordered_map<std::string, std::reference_wrapper<ValueStorage>> shortcuts;
#ifdef DEBUG
        struct {
            uint32_t add, remove,
                     match, match_some,
                     match_iter, match_some_iter,
                     test, test_some;
        } counters;
#endif
    };

    QlobberBase(State* state) : state(state) {
        add_ref();
    }

    QlobberBase() : QlobberBase(new State()) {}
        // TODO:
        // worker threads
        // centro and mqlobber-access-control should check max_words
        //   and max_wildcard_somes to prevent runtime exceptions

    QlobberBase(const Options& options) : QlobberBase() {
        state->options = options;
    }

    QlobberBase(const OptionsOrState<State> options_or_state) {
        if (options_or_state.is_options) {
            state = new State();
            state->options = std::get<0>(options_or_state.data);
        } else {
            state = std::get<1>(options_or_state.data);
        }
        add_ref();
    };

    virtual ~QlobberBase() {
        if (remove_ref() == 0) {
            delete state;
        }
    }

protected:
    void add(const std::string& topic, const Value& val) {
        WriteLock lock(state->rwlock);
        if (state->options.cache_adds) {
            const auto it = state->shortcuts.find(topic);
            if (it != state->shortcuts.end()) {
                return add_value(it->second, val);
            }
        }
        auto& storage = add(val, 0, split(topic, true), state->trie);
        if (state->options.cache_adds) {
            state->shortcuts.emplace(topic, storage);
        }
    }

    void remove(const std::string& topic,
                const std::optional<const RemoveValue>& val) {
        WriteLock lock(state->rwlock);
        if (remove(val, 0, split(topic, false), state->trie) &&
            state->options.cache_adds) {
            state->shortcuts.erase(topic);
        }
    }

    void match(MatchResult& r, const std::string& topic, Context& ctx) {
        ReadLock lock(state->rwlock);
        match(r, 0, split(topic, false), state->trie, ctx);
    }

    std::vector<std::reference_wrapper<const ValueStorage>> match(const std::string& topic, Context& ctx) {
        ReadLock lock(state->rwlock);
        std::vector<std::reference_wrapper<const ValueStorage>> r;
        match(r, 0, split(topic, false), state->trie, ctx);
        return r;
    }

    bool test(const std::string& topic, const TestValue& val) {
        ReadLock lock(state->rwlock);
        return test(val, 0, split(topic, false), state->trie);
    }

    void clear_unlocked() {
        state->shortcuts.clear();
        std::get<0>(state->trie.v)->clear();
    }

    virtual void clear() {
        WriteLock lock(state->rwlock);
        clear_unlocked();
    }

    typedef typename boost::coroutines2::coroutine<Visit<Value>> coro_visit_t;

    typename coro_visit_t::pull_type visit() {
        return typename coro_visit_t::pull_type(
            std::bind(&QlobberBase::generate_visits, this, std::placeholders::_1));
    }

    typename coro_visit_t::push_type restore(bool cache_adds = false) {
        return typename coro_visit_t::push_type(
            std::bind(&QlobberBase::inject, this, std::placeholders::_1, cache_adds));
    }

    typedef typename boost::coroutines2::coroutine<IterValue> coro_iter_t;

    typename coro_iter_t::pull_type match_iter(const std::string& topic, Context& ctx) {
        // "The arguments to bind are copied or moved, and are never passed by
        // reference unless wrapped in std::ref or std::cref."
        // https://en.cppreference.com/w/cpp/utility/functional/bind
        // So it doesn't matter if topic and ctx are destroyed later.
        return typename coro_iter_t::pull_type(
            std::bind(&QlobberBase::generate_values, this, std::placeholders::_1, topic, ctx));
    }

    virtual void add_values(MatchResult& r,
                            const ValueStorage& vals,
                            Context& ctx) = 0;

    uint32_t add_ref() {
        return ++state->ref_count;
    }

    uint32_t remove_ref() {
        return --state->ref_count;
    }

    State* state;

private:
    virtual void initial_value_inserted(const Value& val) {}
    virtual void add_value(ValueStorage& vals, const Value& val) = 0;
    virtual bool remove_value(ValueStorage& vals,
                              const std::optional<const RemoveValue>& val) = 0;
    virtual bool test_values(const ValueStorage& vals,
                             const TestValue& val) = 0;
    virtual void iter_values(typename coro_iter_t::push_type& sink,
                             const ValueStorage& vals,
                             Context& ctx) = 0;
    virtual void visit_values(typename coro_visit_t::push_type& sink,
                              const ValueStorage& storage) = 0;

    ValueStorage& add(const Value& val,
                      const std::size_t i,
                      const std::vector<std::string>& words,
                      const Trie& sub_trie) {
#ifdef DEBUG
        ++state->counters.add;
#endif
        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                auto& storage = std::get<1>(it->second.v);
                add_value(storage, val);
                return storage;
            }

            std::get<0>(sub_trie.v)->emplace(state->options.separator, val);
            initial_value_inserted(val);
            return std::get<1>((*std::get<0>(sub_trie.v))[state->options.separator].v);
        }

        return add(val, i + 1, words, (*std::get<0>(sub_trie.v))[words[i]]);
    }

    bool remove(const std::optional<const RemoveValue>& val,
                const std::size_t i,
                const std::vector<std::string>& words,
                const Trie& sub_trie) {
#ifdef DEBUG
        ++state->counters.remove;
#endif
        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.separator);

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

    template<typename MR>
    void match_some(MR& r,
                    const std::size_t i,
                    const std::vector<std::string>& words,
                    const Trie& st,
                    Context& ctx) {
#ifdef DEBUG
        ++state->counters.match_some;
#endif
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != state->options.separator) {
                for (std::size_t j = i; j < words.size(); ++j) {
                    match(r, j, words, st, ctx);
                }
                break;
            }
        }
    }

    template<typename MR>
    void match(MR& r,
               const std::size_t i,
               const std::vector<std::string>& words,
               const Trie& sub_trie,
               Context& ctx) {
#ifdef DEBUG
        ++state->counters.match;
#endif
        {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.wildcard_some);

            if (it != std::get<0>(sub_trie.v)->end()) {
                // in the common case there will be no more levels...
                match_some(r, i, words, it->second, ctx);
                // and we'll end up matching the rest of the words:
                match(r, words.size(), words, it->second, ctx);
            }
        }

        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                add_values(r, std::get<1>(it->second.v), ctx);
            }
        } else {
            const auto& word = words[i];

            if ((word != state->options.wildcard_one) &&
                (word != state->options.wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match(r, i + 1, words, it->second, ctx);
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(state->options.wildcard_one);

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
#ifdef DEBUG
        ++state->counters.match_some_iter;
#endif
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != state->options.separator) {
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
#ifdef DEBUG
        ++state->counters.match_iter;
#endif
        {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.wildcard_some);

            if (it != std::get<0>(sub_trie.v)->end()) {
                // in the common case there will be no more levels...
                match_some_iter(sink, i, words, it->second, ctx);
                // and we'll end up matching the rest of the words:
                match_iter(sink, words.size(), words, it->second, ctx);
            }
        }

        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                iter_values(sink, std::get<1>(it->second.v), ctx);
            }
        } else {
            const auto& word = words[i];

            if ((word != state->options.wildcard_one) &&
                (word != state->options.wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match_iter(sink, i + 1, words, it->second, ctx);
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(state->options.wildcard_one);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match_iter(sink, i + 1, words, it->second, ctx);
                }
            }
        }
    }

    void generate_values(typename coro_iter_t::push_type& sink,
                         const std::string& topic,
                         Context& ctx) {
        ReadLock lock(state->rwlock);
        match_iter(sink, 0, split(topic, false), state->trie, ctx);
    }

    bool test_some(const TestValue& v,
                   const std::size_t i,
                   const std::vector<std::string>& words,
                   const Trie& st) {
#ifdef DEBUG
        ++state->counters.test_some;
#endif
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != state->options.separator) {
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

    bool test(const TestValue& v,
              const std::size_t i,
              const std::vector<std::string>& words,
              const Trie& sub_trie) {
#ifdef DEBUG
        ++state->counters.test;
#endif
        {
            const auto it = std::get<0>(sub_trie.v)->find(state->options.wildcard_some);

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
            const auto it = std::get<0>(sub_trie.v)->find(state->options.separator);

            if ((it != std::get<0>(sub_trie.v)->end()) &&
                test_values(std::get<1>(it->second.v), v)) {
                return true;
            }
        } else {
            const auto& word = words[i];

            if ((word != state->options.wildcard_one) &&
                (word != state->options.wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if ((it != std::get<0>(sub_trie.v)->end()) &&
                    test(v, i + 1, words, it->second)) {
                    return true;
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(state->options.wildcard_one);

                if ((it != std::get<0>(sub_trie.v)->end()) &&
                    test(v, i + 1, words, it->second)) {
                    return true;
                }
            }
        }

        return false;
    }

    void generate_visits(typename coro_visit_t::push_type& sink) {
        ReadLock lock(state->rwlock);
        struct Saved {
            typename Trie::map_type::const_iterator it, end;
            std::size_t i;
        } cur = {
            std::get<0>(state->trie.v)->cbegin(),
            std::get<0>(state->trie.v)->cend(),
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

            if (cur.it->first == state->options.separator) {
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
        WriteLock lock(state->rwlock);
        struct Saved {
            Saved(std::shared_ptr<Trie> entry,
                  const std::string& key,
                  const std::string& path) :
                entry(entry), key(key), path(path) {}
            std::shared_ptr<Trie> entry;
            std::string key, path;
        };
        std::vector<Saved> saved;
        std::shared_ptr<Trie> entry(&state->trie, [](Trie*) {});
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
                            path += state->options.separator;
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
                        return;
                    }
                    if (entry) {
                        const auto it = std::get<0>(saved.back().entry->v)->emplace(
                            saved.back().key, entry);
                        if (cache_adds &&
                            state->options.cache_adds &&
                            (v.type == Visit<Value>::end_values)) {
                            state->shortcuts.emplace(saved.back().path,
                                                     std::get<1>(it.first->second.v));
                        }
                    }
                    entry = saved.back().entry;
                    path = saved.back().path;
                    saved.pop_back();
                    break;

                case Visit<Value>::start_entries:
                case Visit<Value>::start_values:
                    break;
            }
        }
    }

    void add_values(std::vector<std::reference_wrapper<const ValueStorage>>& r,
                    const ValueStorage& vals,
                    Context& ctx) {
        r.emplace_back(vals);
    }

    void add_word(std::vector<std::string>& words,
                  const std::string& word,
                  const bool adding,
                  std::size_t& wildcard_somes) {
        if (adding &&
            (word == state->options.wildcard_some) &&
            (++wildcard_somes > state->options.max_wildcard_somes)) {
            throw std::length_error("too many wildcard somes");
        }
        words.push_back(word);
        if (words.size() > state->options.max_words) {
            throw std::length_error("too many words");
        }
    }

    std::vector<std::string> split(const std::string& topic,
                                   const bool adding) {
        std::vector<std::string> words;
        std::size_t last = 0;
        std::size_t wildcard_somes = 0;
        while (true) {
            std::size_t next = topic.find(state->options.separator, last);
            if (next == std::string::npos) {
                break;
            }
            add_word(words, topic.substr(last, next - last), adding, wildcard_somes);
            last = next + 1;
        }
        add_word(words, topic.substr(last), adding, wildcard_somes);
        return words;
    }
};
