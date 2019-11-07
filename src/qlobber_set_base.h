#include <unordered_set>
#include "qlobber_container_base.h"

template<typename Value>
class SetStorage : public std::unordered_set<Value> {
public:
    SetStorage(const Value& v) {
        this->insert(v);
    }
};

template<typename Value, typename MatchResult, typename Context>
class QlobberSetBase :
    public QlobberContainerBase<Value, SetStorage, MatchResult, Context> {
public:
    QlobberSetBase() {}

    QlobberSetBase(const Options& options) :
        QlobberContainerBase<Value, SetStorage, MatchResult, Context>(options) {}

    bool test_values(const SetStorage<Value>& existing,
                     const Value& val) override {
        return existing.find(val) != existing.end();
    }

private:
    void add_value(SetStorage<Value>& existing, const Value& val) override {
        existing.insert(val);
    }

    bool remove_value(SetStorage<Value>& existing,
                      const std::optional<const Value>& topic) override {
        if (!topic) {
            return true;
        }

        existing.erase(*topic);

        return existing.empty();
    }
};
