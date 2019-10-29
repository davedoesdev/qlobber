#pragma once

#include <vector>
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
        const auto topic = info[0].As<Napi::String>();
        const auto val = info[1].As<JSValue>();
        this->add(topic, val);
        return info.This();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        if (info.Length() == 1) {
            this->remove(topic, std::nullopt);
        } else {
            const auto val = info[1].As<JSValue>();
            this->remove(topic, val);
        }
        return info.This();
    }

    Napi::Value Match(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        auto r = NewMatchResult(info.Env());
        this->match(r, topic, info.Env());
        return r;
    }

    Napi::Value Test(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        const auto val = info[1].As<JSValue>();
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
std::vector<Napi::ClassPropertyDescriptor<T>> Properties() {
    return {};
}

template<typename T>
void Initialize(Napi::Env env, const char* name, Napi::Object exports) {
    std::vector<Napi::ClassPropertyDescriptor<T>> props {
        T::InstanceMethod("add", &T::Add),
        T::InstanceMethod("remove", &T::Remove),
        T::InstanceMethod("match", &T::Match),
        T::InstanceMethod("test", &T::Test),
        T::InstanceMethod("clear", &T::Clear)
    };

    const auto props2 = Properties<T>();
    props.insert(props.end(), props2.begin(), props2.end());

    exports.Set(name, T::DefineClass(env, name, props));
}
