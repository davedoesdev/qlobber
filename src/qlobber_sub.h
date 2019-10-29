#include <napi.h>
#include "qlobber_sub_base.h"
#include "qlobber_js_base.h"
#include "js_options.h"

struct TopicAndEnv {
    std::optional<const std::string> topic;
    Napi::Env env;
};

class QlobberSub :
    public QlobberSubBase<Napi::Array, const TopicAndEnv>,
    public Napi::ObjectWrap<QlobberSub> {
public:
    QlobberSub(const Napi::CallbackInfo& info) :
        QlobberSubBase<Napi::Array, const TopicAndEnv>(JSOptions(info)), 
        Napi::ObjectWrap<QlobberSub>(info) {}

    virtual ~QlobberSub() {}

    Napi::Value Add(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        auto val = info[1].As<Napi::Object>();
        add(topic, {
            val.Get("clientId").As<Napi::String>(),
            val.Get("topic").As<Napi::String>(),
            static_cast<QoS>(val.Get("qos").As<Napi::Number>().Uint32Value())
        });
        return info.This();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        if (info.Length() == 1) {
            remove(topic, std::nullopt);
        } else {
            auto val = info[1].As<Napi::Object>();
            remove(topic, val.Get("clientId").As<Napi::String>());
        }
        return info.This();
    }

    Napi::Value Match(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        auto r = Napi::Array::New(info.Env());
        match(r, topic, {
            info.Length() == 1 ?
                std::nullopt :
                std::optional<std::string>(info[1].As<Napi::String>()),
            info.Env()
        });
        return r;
    }

    Napi::Value Test(const Napi::CallbackInfo& info) {
        auto topic = info[0].As<Napi::String>();
        auto val = info[1].As<Napi::Object>();
        return Napi::Boolean::New(info.Env(), test(topic, {
            val.Get("clientId").As<Napi::String>(),
            val.Get("topic").As<Napi::String>()
        }));
    }

    Napi::Value Clear(const Napi::CallbackInfo& info) {
        clear();
        return info.This();
    }

    Napi::Value GetSubscriptionsCount(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), subscriptionsCount);
    }

private:
    void add_values(Napi::Array& dest,
                    const SubStorage& existing,
                    const TopicAndEnv& context) override {
        if (!context.topic) {
            for (const auto& clientIdAndQos : existing.clientMap) {
                Napi::Object obj = Napi::Object::New(context.env);
                obj.Set("clientId", clientIdAndQos.first);
                obj.Set("topic", existing.topic);
                obj.Set("qos", static_cast<uint32_t>(clientIdAndQos.second));
                dest.Set(dest.Length(), obj);
            }
        } else if (existing.topic == context.topic.value()) {
            for (const auto& clientIdAndQos : existing.clientMap) {
                Napi::Object obj = Napi::Object::New(context.env);
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
