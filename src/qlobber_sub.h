#include <napi.h>
#include "qlobber_sub_base.h"
#include "qlobber_js_base.h"
#include "js_options.h"

class QlobberSub :
    public QlobberSubBase<Napi::Array, const std::optional<const std::string>>,
    public Napi::ObjectWrap<QlobberSub> {
public:
    QlobberSub(const Napi::CallbackInfo& info) :
        QlobberSubBase<Napi::Array, const std::optional<const std::string>>(JSOptions(info)), 
        Napi::ObjectWrap<QlobberSub>(info) {}

    virtual ~QlobberSub() {}

    Napi::Value Add(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        const auto val = info[1].As<Napi::Object>();
        add(topic, {
            val.Get("clientId").As<Napi::String>(),
            val.Get("topic").As<Napi::String>(),
            static_cast<QoS>(val.Get("qos").As<Napi::Number>().Uint32Value())
        });
        return info.This();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        if (info.Length() == 1) {
            remove(topic, std::nullopt);
        } else {
            const auto val = info[1].As<Napi::Object>();
            remove(topic, val.Get("clientId").As<Napi::String>());
        }
        return info.This();
    }

    Napi::Value Match(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        auto r = Napi::Array::New(info.Env());
        match(r, topic, info.Length() == 1 ?
            std::nullopt :
            std::optional<std::string>(info[1].As<Napi::String>()));
        return r;
    }

    Napi::Value Test(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        const auto val = info[1].As<Napi::Object>();
        return Napi::Boolean::New(info.Env(), test(topic, {
            val.Get("clientId").As<Napi::String>(),
            val.Get("topic").As<Napi::String>()
        }));
    }

    Napi::Value Clear(const Napi::CallbackInfo& info) {
        clear();
        return info.This();
    }

    Napi::Value GetVisitor(const Napi::CallbackInfo& info) {
        return GetVisitorT<QlobberSub, Sub>(this, info);
    }

    Napi::Value VisitNext(const Napi::CallbackInfo& info) {
        return VisitNextT<Sub, Napi::Object>(info);
    }

    Napi::Value GetRestorer(const Napi::CallbackInfo& info) {
        return GetRestorerT<QlobberSub, Sub>(this, info);
    }

    Napi::Value RestoreNext(const Napi::CallbackInfo& info) {
        return RestoreNextT<Sub, Napi::Object>(info);
    }

    Napi::Value GetOptions(const Napi::CallbackInfo& info) {
        return JSOptions::get(info.Env(), options);
    }

    Napi::Value GetSubscriptionsCount(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), subscriptionsCount);
    }

private:
    void add_values(Napi::Array& dest,
                    const SubStorage& existing,
                    const std::optional<const std::string>& topic) override {
        if (!topic) {
            for (const auto& clientIdAndQos : existing.clientMap) {
                Napi::Object obj = Napi::Object::New(dest.Env());
                obj.Set("clientId", clientIdAndQos.first);
                obj.Set("topic", existing.topic);
                obj.Set("qos", static_cast<uint32_t>(clientIdAndQos.second));
                dest.Set(dest.Length(), obj);
            }
        } else if (existing.topic == topic.value()) {
            for (const auto& clientIdAndQos : existing.clientMap) {
                Napi::Object obj = Napi::Object::New(dest.Env());
                obj.Set("clientId", clientIdAndQos.first);
                obj.Set("qos", static_cast<uint32_t>(clientIdAndQos.second));
                dest.Set(dest.Length(), obj);
            }
        }
    }
};

template<>
std::vector<Napi::ClassPropertyDescriptor<QlobberSub>> Properties() {
    return {
        QlobberSub::InstanceAccessor("subscriptionsCount", &QlobberSub::GetSubscriptionsCount, nullptr)
    };
}

template<>
Napi::Value FromValue<Sub, Napi::Object>(const Napi::Env& env, const Sub& sub) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("clientId", sub.clientId);
    obj.Set("topic", sub.topic);
    obj.Set("qos", static_cast<uint32_t>(sub.qos));
    return obj;
}

template<>
Sub ToValue<Sub, Napi::Object>(const Napi::Object& v) {
    return Sub {
        v.Get("clientId").As<Napi::String>(),
        v.Get("topic").As<Napi::String>(),
        static_cast<QoS>(v.Get("qos").As<Napi::Number>().Uint32Value())
    };
}
