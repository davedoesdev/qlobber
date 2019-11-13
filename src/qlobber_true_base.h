#include "qlobber_base.h"

struct TrueValue {};

struct TrueStorage {
    TrueStorage(const TrueValue&) {}
};

template<typename Value,
         typename MatchResult,
         typename Context,
         typename TestValue,
         typename IterValue>
class QlobberTrueBase;

template<>
class QlobberTrueBase<TrueValue, bool, const std::nullptr_t, std::nullptr_t, TrueValue> :
    public QlobberBase<TrueValue,
                       TrueStorage,
                       std::nullptr_t,
                       bool,
                       const std::nullptr_t,
                       std::nullptr_t> {
public:
    QlobberTrueBase() {}

    QlobberTrueBase(const Options& options) :
        QlobberBase<TrueValue,
                    TrueStorage,
                    std::nullptr_t, 
                    bool,
                    const std::nullptr_t,
                    std::nullptr_t>(options) {}

protected:
    void add_values(bool& r, const TrueStorage&, const std::nullptr_t&) override {
        r = true;
    }

private:
    void add_value(TrueStorage&, const TrueValue&) override {}

    bool remove_value(TrueStorage&,
                      const std::optional<const std::nullptr_t>&) override {
        return true;
    }

    bool test_values(const TrueStorage& existing,
                     const std::nullptr_t&) override {
        return true;
    }

    void iter_values(typename boost::coroutines2::coroutine<TrueValue>::push_type& sink,
                     const TrueStorage&,
                     const std::nullptr_t&) override {
        sink(TrueValue());
    }

    void visit_values(typename boost::coroutines2::coroutine<Visit<TrueValue>>::push_type& sink,
                      const TrueStorage&) override {
        sink({
            Visit<TrueValue>::value,
            VisitData<TrueValue> {
                std::variant<std::string, TrueValue>(
                    std::in_place_index<1>, TrueValue())
            }
        });
    }
};
