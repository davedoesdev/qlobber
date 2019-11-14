#include <napi.h>
#include "qlobber_true_base.h"
#include "qlobber_js_base.h"

class QlobberTrue :
    public QlobberJSCommon<TrueValue,
                           Napi::Boolean, 
                           bool,
                           const std::nullptr_t,
                           QlobberTrueBase,
                           std::nullptr_t,
                           std::nullptr_t>,
    public Napi::ObjectWrap<QlobberTrue> {
public:
    QlobberTrue(const Napi::CallbackInfo& info) :
        QlobberJSCommon<TrueValue,
                        Napi::Boolean,
                        bool,
                        const std::nullptr_t,
                        QlobberTrueBase,
                        std::nullptr_t,
                        std::nullptr_t>(info),
        Napi::ObjectWrap<QlobberTrue>(info) {}

    virtual ~QlobberTrue() {}

private:
    std::nullptr_t get_context(const Napi::CallbackInfo&) override {
        return nullptr;
    }

    bool NewMatchResult(const Napi::Env&) override {
        return false;
    }

    TrueValue get_add_value(const Napi::CallbackInfo&) override {
        return TrueValue();
    }

    std::optional<const std::nullptr_t> get_remove_value(const Napi::CallbackInfo&) override {
        return std::nullopt;
    }

    std::nullptr_t get_test_value(const Napi::CallbackInfo&) override {
        return nullptr;
    }
};

template<>
Napi::Value MatchResultValue<bool>(const Napi::Env& env, bool& r) {
    return Napi::Boolean::New(env, r);
}

template<>
Napi::Boolean FromValue<TrueValue, Napi::Boolean>(const Napi::Env& env, const TrueValue&) {
    return Napi::Boolean::New(env, true);
}

template<>
TrueValue ToValue<TrueValue, Napi::Boolean>(const Napi::Boolean& v) {
    return TrueValue();
}
