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
        typedef std::unordered_map<std::string, struct Trie> map_type;
        typedef std::unique_ptr<map_type> map_ptr;
        std::variant<ValueStorage, map_ptr> v;
    } trie;

    ValueStorage& _add(const Value& val,
                       const uint64_t i,
                       const std::vector<std::string>& words,
                       const struct Trie& sub_trie) {
        if (i == words.size()) {
            const auto it = std::get<1>(sub_trie.v)->find(separator);

            if (it != std::get<1>(sub_trie.v)->end()) {
                add_value(std::get<0>(it->second.v), val);
                return std::get<0>(it->second.v);
            }

            (*std::get<1>(sub_trie.v))[separator].v = initial_value(val);
            return std::get<0>((*std::get<1>(sub_trie.v))[separator].v);
        }

        return _add(val, i + 1, words, (*std::get<1>(sub_trie.v))[words[i]]);
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

    virtual ValueStorage initial_value(const Value& val) = 0;
    virtual void add_value(ValueStorage& vals, const Value& val) = 0;
};
