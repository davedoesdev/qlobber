#include "qlobber_base.h"

struct TrueValue {};

struct TrueStorage {
    TrueStorage(const TrueValue&) {}
};

class QlobberTrueBase :
    public QlobberBase<TrueValue,
                       TrueStorage,
                       std::nullptr_t,
                       std::nullptr_t,
                       const std::nullptr_t,
                       std::nullptr_t> {
public:
    QlobberTrueBase() {}

    QlobberTrueBase(const Options& options) :
        QlobberBase<TrueValue,
                    TrueStorage,
                    std::nullptr_t, 
                    std::nullptr_t,
                    const std::nullptr_t,
                    std::nullptr_t>(options) {}

    bool test_values(const TrueStorage& existing,
                     const std::nullptr_t&) override {
        return true;
    }

private:
    void add_value(TrueStorage&, const TrueValue&) override {}

    bool remove_value(TrueStorage&,
                      const std::optional<const std::nullptr_t>&) override {
        return true;
    }

    void add_values(std::nullptr_t&, const TrueStorage&, const std::nullptr_t&) {}
};

template<>
void VisitValues<TrueValue, TrueStorage>(
    const TrueStorage&,
    typename boost::coroutines2::coroutine<Visit<TrueValue>>::push_type& sink) {
    sink({
        Visit<TrueValue>::value,
        VisitData<TrueValue> {
            std::variant<std::string, TrueValue>(
                std::in_place_index<1>, TrueValue())
        }
    });
}
