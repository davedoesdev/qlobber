#include "qlobber_base.h"

struct TrueStorage {
    TrueStorage(const std::nullptr_t&) {}
};

class QlobberTrueBase :
    public QlobberBase<std::nullptr_t,
                       TrueStorage,
                       std::nullptr_t,
                       std::nullptr_t,
                       const std::nullptr_t,
                       std::nullptr_t> {
public:
    QlobberTrueBase() {}

    QlobberTrueBase(const Options& options) :
        QlobberBase<std::nullptr_t,
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
    void add_value(TrueStorage&, const std::nullptr_t&) override {}

    bool remove_value(TrueStorage&,
                      const std::optional<const std::nullptr_t>&) override {
        return true;
    }

    void add_values(std::nullptr_t&, const TrueStorage&, const std::nullptr_t&) {}
};

template<>
void VisitValues<std::nullptr_t, TrueStorage>(
    const TrueStorage&,
    typename boost::coroutines2::coroutine<Visit<std::nullptr_t>>::push_type& sink) {
    sink({
        Visit<std::nullptr_t>::value,
        VisitData<std::nullptr_t> {
            0,
            std::variant<std::string, std::nullptr_t>(
                std::in_place_index<1>, nullptr)
        }
    });
}
