#include <napi.h>
#include "qlobber_sub_base.h"
#include "qlobber_js_base.h"

template<>
Sub ToValue<Sub, Napi::Object>(const Napi::Object& v) {
    return {
        v.Get("clientId").As<Napi::String>(),
        v.Get("topic").As<Napi::String>(),
        static_cast<QoS>(v.Get("qos").As<Napi::Number>().Uint32Value()),
        static_cast<RetainHandling>(v.Get("rh").As<Napi::Number>().Uint32Value()),
        v.Get("rap").As<Napi::Boolean>(),
        v.Get("nl").As<Napi::Boolean>()
    };
}

void SetSubscriptionProperties(Napi::Object& obj, const SubStorageNode& node) {
    obj.Set("qos", static_cast<uint32_t>(node.qos));
    obj.Set("rh", static_cast<uint32_t>(node.rh));
    obj.Set("rap", node.rap);
    obj.Set("nl", node.nl);
}

class QlobberSub :
    public QlobberJSCommon<Sub,
                           Napi::Object, 
                           Napi::Array,
                           const std::optional<const std::string>,
                           QlobberSubBase,
                           std::string,
                           SubTest,
                           IterSub>,
    public Napi::ObjectWrap<QlobberSub> {
public:
    QlobberSub(const Napi::CallbackInfo& info) :
        QlobberJSCommon<Sub,
                        Napi::Object,
                        Napi::Array,
                        const std::optional<const std::string>,
                        QlobberSubBase,
                        std::string,
                        SubTest,
                        IterSub>(info),
        Napi::ObjectWrap<QlobberSub>(info) {}

    virtual ~QlobberSub() {}

    Napi::Value GetSubscriptionsCount(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), state->subscriptionsCount);
    }

private:
    std::optional<const std::string> get_context(const Napi::CallbackInfo& info) override {
        if ((info.Length() > 1) && info[1].IsString()) {
            return std::optional<std::string>(info[1].As<Napi::String>());
        }
        return std::nullopt;
    }

    Napi::Array NewMatchResult(const Napi::Env& env) override {
        return Napi::Array::New(env);
    }

    Sub get_add_value(const Napi::CallbackInfo& info) override {
        return ToValue<Sub, Napi::Object>(info[1].As<Napi::Object>());
    }

    std::optional<const std::string> get_remove_value(const Napi::Value& v) override {
        return v.As<Napi::Object>().Get("clientId").As<Napi::String>();
    }

    SubTest get_test_value(const Napi::CallbackInfo& info) override {
        const auto val = info[1].As<Napi::Object>();
        return {
            val.Get("clientId").As<Napi::String>(),
            val.Get("topic").As<Napi::String>()
        };
    }

    void add_values(Napi::Array& dest,
                    const SubStorage& existing,
                    const std::optional<const std::string>& topic) override {
        if (!topic) {
            for (const auto& clientIdAndNode : existing.clientMap) {
                Napi::Object obj = Napi::Object::New(dest.Env());
                obj.Set("clientId", clientIdAndNode.first);
                obj.Set("topic", existing.topic);
                SetSubscriptionProperties(obj, clientIdAndNode.second);
                dest.Set(dest.Length(), obj);
            }
        } else if (existing.topic == topic.value()) {
            for (const auto& clientIdAndNode : existing.clientMap) {
                Napi::Object obj = Napi::Object::New(dest.Env());
                obj.Set("clientId", clientIdAndNode.first);
                SetSubscriptionProperties(obj, clientIdAndNode.second);
                dest.Set(dest.Length(), obj);
            }
        }
    }

    Napi::Object get_object() override {
        return Value();
    }

};

template<>
std::vector<Napi::ClassPropertyDescriptor<QlobberSub>> Properties() {
    return {
        QlobberSub::InstanceAccessor<&QlobberSub::GetSubscriptionsCount>("subscriptionsCount")
    };
}

template<>
Napi::Object FromValue<Sub, Napi::Object>(const Napi::Env& env, const Sub& sub) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("clientId", sub.clientId);
    obj.Set("topic", sub.topic);
    SetSubscriptionProperties(obj, sub.node);
    return obj;
}

template<>
Napi::Object FromValue<IterSub, Napi::Object>(const Napi::Env& env, const IterSub& sub) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("clientId", sub.clientId);
    if (sub.topic) {
        obj.Set("topic", sub.topic.value());
    }
    SetSubscriptionProperties(obj, sub.node);
    return obj;
}
