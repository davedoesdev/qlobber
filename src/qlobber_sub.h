#include <napi.h>
#include "qlobber_sub_base.h"
#include "qlobber_js_base.h"
#include "js_options.h"

struct IterSub {
    std::string clientId;
    std::optional<std::string> topic;
    QoS qos;
};

class QlobberSub :
    public QlobberSubBase<Napi::Array, const std::optional<const std::string>, IterSub>,
    public Napi::ObjectWrap<QlobberSub> {
public:
    QlobberSub(const Napi::CallbackInfo& info) :
        QlobberSubBase<Napi::Array, const std::optional<const std::string>, IterSub>(
            JSOptions(info)),
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
        auto r = NewMatchResult(info.Env());
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

    Napi::Value MatchIter(const Napi::CallbackInfo& info) {
        return MatchIterT<QlobberSub, IterSub, const std::optional<const std::string>>(
            this, info, info.Length() == 1 ?
                std::nullopt :
                std::optional<std::string>(info[1].As<Napi::String>())
        );
    }

    Napi::Value MatchNext(const Napi::CallbackInfo& info) {
        return MatchNextT<IterSub, Napi::Object>(info);
    }

    Napi::Value GetSubscriptionsCount(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), subscriptionsCount);
    }

    // for tests

    friend Napi::Value GetShortcutsT<QlobberSub, const std::optional<const std::string>>(
        QlobberSub*, const Napi::CallbackInfo&, const std::optional<const std::string>&);

    Napi::Value GetShortcuts(const Napi::CallbackInfo& info) {
        return GetShortcutsT< QlobberSub, const std::optional<const std::string>>(
            this, info, std::nullopt);
    }

private:
    Napi::Array NewMatchResult(const Napi::Env& env) {
        return Napi::Array::New(env);
    }

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
Napi::Object FromValue<Sub, Napi::Object>(const Napi::Env& env, const Sub& sub) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("clientId", sub.clientId);
    obj.Set("topic", sub.topic);
    obj.Set("qos", static_cast<uint32_t>(sub.qos));
    return obj;
}

template<>
Napi::Object FromValue<IterSub, Napi::Object>(const Napi::Env& env, const IterSub& sub) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("clientId", sub.clientId);
    if (sub.topic) {
        obj.Set("topic", sub.topic.value());
    }
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

template<>
void IterValues<IterSub, SubStorage, const std::optional<const std::string>>(
    const SubStorage& storage,
    const std::optional<const std::string>& topic,
    typename boost::coroutines2::coroutine<IterSub>::push_type& sink) {
    if (!topic) {
        for (const auto& clientIdAndQos : storage.clientMap) {
            sink(IterSub {
                clientIdAndQos.first,
                std::optional<std::string>(storage.topic),
                clientIdAndQos.second
            });
        }
    } else if (storage.topic == topic.value()) {
        for (const auto& clientIdAndQos : storage.clientMap) {
            sink(IterSub {
                clientIdAndQos.first,
                std::nullopt,
                clientIdAndQos.second
            });
        }
    }
}
