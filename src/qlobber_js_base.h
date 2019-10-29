#pragma once

#include "js_options.h"

template<typename Value,
         typename JSValue,
         typename MatchResult,
         template<typename, typename, typename> typename Base>
class QlobberJSBase :
    public Base<Value, MatchResult, const Napi::Env> {
public:
    QlobberJSBase(const Napi::CallbackInfo& info) :
        Base<Value, MatchResult, const Napi::Env>(JSOptions(info)) {}

    virtual ~QlobberJSBase() {}

    Napi::Value Add(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        auto val = info[1].As<JSValue>();
        this->add(topic, val);
        return info.This();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        if (info.Length() == 1) {
            this->remove(topic, std::nullopt);
        } else {
            auto val = info[1].As<JSValue>();
            this->remove(topic, val);
        }
        return info.This();
    }

    Napi::Value Match(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        auto r = NewMatchResult(info.Env());
        this->match(r, topic, info.Env());
        return r;
    }

    Napi::Value Test(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        auto val = info[1].As<JSValue>();
        return Napi::Boolean::New(info.Env(), this->test(topic, val));
    }

    Napi::Value Clear(const Napi::CallbackInfo& info) {
        this->clear();
        return info.This();
    }

protected:
private:
    virtual MatchResult NewMatchResult(const Napi::Env& env) = 0;
};

template<typename T>
void Initialize(Napi::Env env, const char* name, Napi::Object exports) {
    exports.Set(name, T::DefineClass(env, name, {
        T::InstanceMethod("add", &T::Add),
        T::InstanceMethod("remove", &T::Remove),
        T::InstanceMethod("match", &T::Match),
        T::InstanceMethod("test", &T::Test),
        T::InstanceMethod("clear", &T::Clear)
    }));
}
