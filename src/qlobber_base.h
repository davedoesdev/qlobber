#include <string>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <variant>

template<typename Value, typename ValueStorage>
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
        _add(val, 0, split(topic), trie);
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

    ValueStorage& _add(const Value& val,
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

        return _add(val, i + 1, words, (*std::get<0>(sub_trie.v))[words[i]]);
    }

    std::vector<std::string> split(const std::string& topic) {
        std::vector<std::string> words;
        std::stringstream stream(topic);
        std::string word;
        while (std::getline(stream, word, separator[0])) {
            words.push_back(word);
        }
        return words;
    }

    virtual void initial_value_inserted(const Value&) {}
    virtual void add_value(ValueStorage& vals, const Value& val) = 0;
};
