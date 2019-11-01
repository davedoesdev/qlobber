#pragma once

#include <vector>
#include "js_options.h"

template<typename Value>
struct Visitor {
    typedef typename boost::coroutines2::coroutine<Visit<Value>> coro_t;
    typedef typename coro_t::pull_type pull_t;
    Visitor(pull_t source) :
        source(std::move(source)),
        it(begin(this->source)),
        it_end(end(this->source)) {}
    pull_t source;
    typename pull_t::iterator it, it_end;
};

template<typename T, typename Value>
Napi::Value GetVisitorT(T* obj, const Napi::CallbackInfo& info) {
    return Napi::External<Visitor<Value>>::New(
        info.Env(),
        new Visitor<Value>(obj->visit()),
        [](Napi::Env, Visitor<Value>* visitor) {
            delete visitor;
        });
}

template<typename Value, typename JSValue>
Napi::Value FromValue(const Napi::Env& env, const Value& v) {
    return JSValue::New(env, v);
}

template<typename Value, typename JSValue>
Napi::Value VisitNextT(const Napi::CallbackInfo& info) {
    const auto visitor = info[0].As<Napi::External<Visitor<Value>>>().Data();
    if (visitor->it == visitor->it_end) {
        return info.Env().Undefined();
    }

    Napi::Object r = Napi::Object::New(info.Env());
    switch (visitor->it->type) {
        case Visit<Value>::start_entries:
            r.Set("type", "start_entries");
            break;

        case Visit<Value>::entry:
            r.Set("type", "entry");
            r.Set("i", Napi::Number::New(info.Env(), visitor->it->v->i));
            r.Set("key", Napi::String::New(info.Env(), std::get<0>(visitor->it->v->data)));
            break;

        case Visit<Value>::end_entries:
            r.Set("type", "end_entries");
            break;

        case Visit<Value>::start_values:
            r.Set("type", "start_values");
            break;

        case Visit<Value>::value:
            r.Set("type", "value");
            r.Set("i", Napi::Number::New(info.Env(), visitor->it->v->i));
            r.Set("value", FromValue<Value, JSValue>(
                info.Env(), std::get<1>(visitor->it->v->data)));
            break;

        case Visit<Value>::end_values:
            r.Set("type", "end_values");
            break;
    }

    ++visitor->it;
    return r;
}

template<typename Value,
         typename JSValue,
         typename MatchResult,
         template<typename, typename, typename> typename Base>
class QlobberJSBase :
    public Base<Value, MatchResult, const std::nullptr_t> {
public:
    QlobberJSBase(const Napi::CallbackInfo& info) :
        Base<Value, MatchResult, const std::nullptr_t>(JSOptions(info)) {}

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
        const auto ctx = nullptr;
        this->match(r, topic, ctx);
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

    Napi::Value GetVisitor(const Napi::CallbackInfo& info) {
        return GetVisitorT<QlobberJSBase, Value>(this, info);
    }

    Napi::Value VisitNext(const Napi::CallbackInfo& info) {
        return VisitNextT<Value, JSValue>(info);
    }

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
        T::InstanceMethod("clear", &T::Clear),
        T::InstanceMethod("get_visitor", &T::GetVisitor),
        T::InstanceMethod("visit_next", &T::VisitNext)
    };

    const auto props2 = Properties<T>();
    props.insert(props.end(), props2.begin(), props2.end());

    exports.Set(name, T::DefineClass(env, name, props));
}
