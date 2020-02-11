#include <unordered_set>
#include "qlobber_container_base.h"

template<typename Value>
class SetStorage : public std::unordered_set<Value> {
public:
    SetStorage(const Value& v) {
        this->insert(v);
    }
};

template<typename Value,
         typename MatchResult,
         typename Context,
         typename RemoveValue,
         typename TestValue,
         typename IterValue>
class QlobberSetBase;

template<typename Value,
         typename MatchResult,
         typename Context>
class QlobberSetBase<Value, MatchResult, Context, Value, Value, Value> :
    public QlobberContainerBase<Value,
                                SetStorage,
                                MatchResult,
                                Context> {
public:
    QlobberSetBase() {}

    QlobberSetBase(const Options& options) :
        QlobberContainerBase<Value,
                             SetStorage,
                             MatchResult,
                             Context>(options) {}

    QlobberSetBase(const OptionsOrState<typename QlobberSetBase::State>& options_or_state) :
        QlobberContainerBase<Value,
                             SetStorage,
                             MatchResult,
                             Context>(options_or_state) {}

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

    bool test_values(const SetStorage<Value>& existing,
                     const Value& val) override {
        return existing.find(val) != existing.end();
    }
};
