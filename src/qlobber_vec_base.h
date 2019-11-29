#include <vector>
#include "qlobber_container_base.h"

template<typename Value>
class VecStorage : public std::vector<Value> {
public:
    VecStorage(const Value& v) {
        this->push_back(v);
    }
};

template<typename Value,
         typename MatchResult,
         typename Context,
         typename RemoveValue,
         typename TestValue,
         typename IterValue>
class QlobberVecBase;

template<typename Value,
         typename MatchResult,
         typename Context>
class QlobberVecBase<Value, MatchResult, Context, Value, Value, Value> :
    public QlobberContainerBase<Value,
                                VecStorage,
                                MatchResult,
                                Context> {
public:
    QlobberVecBase() {}

    QlobberVecBase(const Options& options) :
        QlobberContainerBase<Value,
                             VecStorage,
                             MatchResult,
                             Context>(options) {}

    QlobberVecBase(const OptionsOrState<typename QlobberVecBase::State>& options_or_state) :
        QlobberContainerBase<Value,
                             VecStorage,
                             MatchResult,
                             Context>(options_or_state) {}

private:
    void add_value(VecStorage<Value>& existing, const Value& val) override {
        existing.push_back(val);
    }
};
