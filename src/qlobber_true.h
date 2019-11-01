#include <napi.h>
#include "qlobber_true_base.h"
#include "qlobber_js_base.h"
#include "js_options.h"

class QlobberTrue :
    public QlobberTrueBase,
    public Napi::ObjectWrap<QlobberTrue> {
public:
    QlobberTrue(const Napi::CallbackInfo& info) :
        QlobberTrueBase(JSOptions(info)),
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

    Napi::Value Clear(const Napi::CallbackInfo& info) {
        this->clear();
        return info.This();
    }

    Napi::Value GetVisitor(const Napi::CallbackInfo& info) {
        return GetVisitorT<QlobberTrue, TrueValue>(this, info);
    }

    Napi::Value VisitNext(const Napi::CallbackInfo& info) {
        return VisitNextT<TrueValue, Napi::Boolean>(info);
    }
};

template<>
Napi::Value FromValue<TrueValue, Napi::Boolean>(const Napi::Env& env, const TrueValue&) {
    return Napi::Boolean::New(env, true);
}
