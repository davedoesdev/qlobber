#include <string>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>

template<typename Value,
         typename ValueStorage,
         typename MatchResult,
         typename Context>
class QlobberBase {
public:
    QlobberBase(const char separator = '.',
                const char wildcard_one = '*',
                const char wildcard_some = '#') :
        separator(1, separator),
        wildcard_one(1, wildcard_one),
        wildcard_some(1, wildcard_some) {
        // TODO: cache_adds/shortcuts        
    }

    void add(const std::string& topic, const Value& val) {
        add(val, 0, split(topic), trie);
    }

    void remove(const std::string& topic,
                const std::optional<const Value>& val) {
        remove(val, 0, split(topic), trie);
    }

    MatchResult match(const std::string& topic,
                      Context& ctx) {
        MatchResult r;
        match(r, 0, split(topic), trie, ctx);
        return r;
    }

private:
    std::string separator;
    std::string wildcard_one;
    std::string wildcard_some;

    struct Trie {
        Trie() {}
        Trie(const Value& val) : v(std::in_place_index<1>, val) {}
        typedef std::unordered_map<std::string, struct Trie> map_type;
        typedef std::unique_ptr<map_type> map_ptr;
        std::variant<map_ptr, ValueStorage> v;
    } trie;

    ValueStorage& add(const Value& val,
                      const std::size_t i,
                      const std::vector<std::string>& words,
                      const struct Trie& sub_trie) {
        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                ValueStorage& storage = std::get<1>(it->second.v);
                add_value(storage, val);
                return storage;
            }

            std::get<0>(sub_trie.v)->emplace(separator, val);
            initial_value_inserted(val);
            return std::get<1>((*std::get<0>(sub_trie.v))[separator].v);
        }

        return add(val, i + 1, words, (*std::get<0>(sub_trie.v))[words[i]]);
    }

    bool remove(const std::optional<const Value>& val,
                const std::size_t i,
                const std::vector<std::string>& words,
                const struct Trie& sub_trie) {
        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(separator);

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
                    const struct Trie& st,
                    Context& ctx) {
        for (const auto& w : *std::get<0>(st.v)) {
            if (w.first != separator) {
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
               const struct Trie& sub_trie,
               Context& ctx) {
        {
            const auto it = std::get<0>(sub_trie.v)->find(wildcard_some);

            if (it != std::get<0>(sub_trie.v)->end()) {
                // in the common case there will be no more levels...
                match_some(r, i, words, it->second, ctx);
                // and we'll end up matching the rest of the words:
                match(r, words.size(), words, it->second, ctx);
            }
        }

        if (i == words.size()) {
            const auto it = std::get<0>(sub_trie.v)->find(separator);

            if (it != std::get<0>(sub_trie.v)->end()) {
                add_values(r, std::get<1>(it->second.v), ctx);
            }
        } else {
            const auto& word = words[i];

            if ((word != wildcard_one) && (word != wildcard_some)) {
                const auto it = std::get<0>(sub_trie.v)->find(word);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match(r, i + 1, words, it->second, ctx);
                }
            }

            if (!word.empty()) {
                const auto it = std::get<0>(sub_trie.v)->find(wildcard_one);

                if (it != std::get<0>(sub_trie.v)->end()) {
                    match(r, i + 1, words, it->second, ctx);
                }
            }
        }
    }

    std::vector<std::string> split(const std::string& topic) {
        std::vector<std::string> words;
        std::stringstream stream(topic);
        std::string word;
        while (std::getline(stream, word, separator[0])) {
            words.push_back(std::move(word));
        }
        return words;
    }

    virtual void initial_value_inserted(const Value& val) {}
    virtual void add_value(ValueStorage& vals, const Value& val) = 0;
    virtual bool remove_value(ValueStorage& vals,
                              const std::optional<const Value>& val) = 0;
    virtual void add_values(MatchResult& r,
                            const ValueStorage& vals,
                            Context& ctx) = 0;
};
