#pragma once

#include "qlobber_base.h"

template<typename Value, template<typename> typename Storage, typename MatchResult, typename Context>
class QlobberContainerBase :
    public QlobberBase<Value, Storage<Value>, Value, MatchResult, Context, Value> {
public:
    QlobberContainerBase() {}

    QlobberContainerBase(const Options& options) :
        QlobberBase<Value, Storage<Value>, Value, MatchResult, Context, Value>(options) {}

    bool test_values(const Storage<Value>& existing,
                     const Value& val) override {
        return std::find(existing.begin(), existing.end(), val) != existing.end();
    }

private:
    bool remove_value(Storage<Value>& existing,
                      const std::optional<const Value>& topic) override {
        if (!topic) {
            return true;
        }

        const auto it = std::find(existing.begin(), existing.end(), topic);

        if (it != existing.end()) {
            existing.erase(it);
        }

        return existing.empty();
    }
};
