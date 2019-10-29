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

private:
    void add_value(SetStorage<Value>& existing, const Value& val) override {
        existing.insert(val);
    }
};
