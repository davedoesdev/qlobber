#include <napi.h>
#include "qlobber_true_base.h"
#include "qlobber_js_base.h"

class QlobberTrue :
    public QlobberJSCommon<TrueValue,
                           Napi::Boolean, 
                           bool,
                           const std::nullptr_t,
                           QlobberTrueBase>,
    public Napi::ObjectWrap<QlobberTrue> {
public:
    QlobberTrue(const Napi::CallbackInfo& info) :
        QlobberJSCommon<TrueValue,
                        Napi::Boolean,
                        bool,
                        const std::nullptr_t,
                        QlobberTrueBase>(info),
        Napi::ObjectWrap<QlobberTrue>(info) {}

    virtual ~QlobberTrue() {}

    Napi::Value Add(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        add(topic, TrueValue());
        return info.This();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        remove(topic, std::nullopt);
        return info.This();
    }

    Napi::Value Match(const Napi::CallbackInfo& info) {
        return Test(info);
    }

    Napi::Value Test(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        return Napi::Boolean::New(info.Env(), this->test(topic, nullptr));
    }

    std::nullptr_t get_context(const Napi::CallbackInfo& info) override {
        return nullptr;
    }

private:
    bool NewMatchResult(const Napi::Env&) override {
        return false;
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
