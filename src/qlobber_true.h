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

    Napi::Value GetRestorer(const Napi::CallbackInfo& info) {
        return GetRestorerT<QlobberTrue, TrueValue>(this, info);
    }

    Napi::Value RestoreNext(const Napi::CallbackInfo& info) {
        return RestoreNextT<TrueValue, Napi::Boolean>(info);
    }

    Napi::Value GetOptions(const Napi::CallbackInfo& info) {
        return JSOptions::get(info.Env(), options);
    }

    // for tests

    friend Napi::Value GetShortcutsT<QlobberTrue, const std::nullptr_t>(
        QlobberTrue*, const Napi::CallbackInfo&, const std::nullptr_t&);

    Napi::Value GetShortcuts(const Napi::CallbackInfo& info) {
        return GetShortcutsT<QlobberTrue, const std::nullptr_t>(this, info, nullptr);
    }

private:
    Napi::Boolean NewMatchResult(const Napi::Env& env) {
        return Napi::Boolean::New(env, true);
    }

    void add_values(Napi::Boolean&, const TrueStorage&, const std::nullptr_t&) {}
};

template<>
Napi::Value FromValue<TrueValue, Napi::Boolean>(const Napi::Env& env, const TrueValue&) {
    return Napi::Boolean::New(env, true);
}

template<>
TrueValue ToValue<TrueValue, Napi::Boolean>(const Napi::Boolean& v) {
    return TrueValue();
}
