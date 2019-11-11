#pragma once

#include "qlobber_base.h"

template<typename Value,
         template<typename> typename Storage,
         typename MatchResult,
         typename Context>
class QlobberContainerBase :
    public QlobberBase<Value,
                       Storage<Value>,
                       Value,
                       MatchResult,
                       Context,
                       Value> {
public:
    QlobberContainerBase() {}

    QlobberContainerBase(const Options& options) :
        QlobberBase<Value,
                    Storage<Value>,
                    Value,
                    MatchResult,
                    Context,
                    Value>(options) {}

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

    bool test_values(const Storage<Value>& existing,
                     const Value& val) override {
        return std::find(existing.begin(), existing.end(), val) != existing.end();
    }

    void iter_values(typename boost::coroutines2::coroutine<Value>::push_type& sink,
                     const Storage<Value>& vals,
                     Context& ctx) override {
        for (const auto& v : vals) {
            sink(v);
        }
    }

    void visit_values(typename boost::coroutines2::coroutine<Visit<Value>>::push_type& sink,
                      const Storage<Value>& vals) override {
        for (const auto& v : vals) {
            sink({
                Visit<Value>::value,
                VisitData<Value> {
                    std::variant<std::string, Value>(
                        std::in_place_index<1>, v)
                }
            });
        }
    }
};
