#include <vector>
#include <optional>
#include <napi.h>
#include "qlobber_base.h"

enum QoS {
    at_most_once = 0,
    at_least_once = 1,
    exactly_once = 2
};

struct Sub {
    std::string clientId;
    std::string topic;
    QoS qos;
};

struct SubStorage {
    SubStorage(const Sub& sub) :
        topic(sub.topic) {
        clientMap.insert_or_assign(sub.clientId, sub.qos);
    }
    std::string topic;
    std::unordered_map<std::string, QoS> clientMap;
};

struct SubResult {
    SubResult(const std::string& clientId,
              const std::string& topic,
              const QoS qos) :
        clientId(clientId),
        topic(topic),
        qos(qos) {}

    SubResult(const std::string& clientId,
              const QoS qos) :
        clientId(clientId),
        qos(qos) {}

    std::string clientId;
    std::optional<std::string> topic;
    QoS qos;
};

struct SubTest {
    std::string clientId;
    std::string topic;
};

template<typename MatchResult, typename Context>
class QlobberSubBase :
    public QlobberBase<Sub, SubStorage, std::string, MatchResult, Context, SubTest> {
public:
    // Returns whether client is last subscriber to topic
    bool test_values(const SubStorage& existing,
                     const SubTest& val) override {
        return (existing.topic == val.topic) &&
               (existing.clientMap.size() == 1) &&
               (existing.clientMap.count(val.clientId) == 1);
    }

    void clear() {
        subscriptionsCount = 0;
        QlobberBase<Sub, SubStorage, std::string, MatchResult, Context, SubTest>::clear();
    }

protected:
    std::size_t subscriptionsCount = 0;

private:
    void initial_value_inserted(const Sub& sub) override {
        ++subscriptionsCount;
    }

    void add_value(SubStorage& existing, const Sub& sub) override {
        if (existing.clientMap.insert_or_assign(sub.clientId, sub.qos).second) {
            ++subscriptionsCount;
        }
    }

    bool remove_value(SubStorage& vals,
                      const std::optional<const std::string>& clientId) override {
        if (vals.clientMap.erase(clientId.value()) > 0) {
            --subscriptionsCount;
        }
        return vals.clientMap.empty();
    }
};

struct TopicAndEnv {
    std::optional<const std::string> topic;
    Napi::Env env;
};

class QlobberSub :
    public QlobberSubBase<Napi::Array, const TopicAndEnv>,
    public Napi::ObjectWrap<QlobberSub> {
public:
    QlobberSub(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<QlobberSub>(info) {
        if (info.Length() == 1) {
            auto options = info[0].As<Napi::Object>();
            if (options.Has("separator")) {
                separator = options.Get("separator").As<Napi::String>();
            }
            if (options.Has("wildcard_one")) {
                wildcard_one = options.Get("wildcard_one").As<Napi::String>();
            }
            if (options.Has("wildcard_some")) {
                wildcard_some = options.Get("wildcard_some").As<Napi::String>();
            }
        }
    }

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

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
        exports.Set("QlobberSubNative", DefineClass(env, "QlobberSubNative", {
            InstanceMethod("add", &QlobberSub::Add),
            InstanceMethod("remove", &QlobberSub::Remove),
            InstanceMethod("match", &QlobberSub::Match),
            InstanceMethod("test", &QlobberSub::Test),
            InstanceMethod("clear", &QlobberSub::Clear),
            InstanceAccessor("subscriptionsCount", &QlobberSub::GetSubscriptionsCount, nullptr)
        }));

        return exports;
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

/*
class QlobberSub :
    public QlobberSubBase<std::vector<SubResult>,
                          const std::optional<const std::string>> {
private:
    void add_values(std::vector<SubResult>& dest,
                    const SubStorage& existing,
                    const std::optional<const std::string>& topic) {
        if (!topic) {
            for (const auto& clientIdAndQos : existing.clientMap) {
                dest.emplace_back(
                    clientIdAndQos.first,
                    existing.topic,
                    clientIdAndQos.second
                );
            }
        } else if (existing.topic == topic.value()) {
            for (const auto& clientIdAndQos : existing.clientMap) {
                dest.emplace_back(
                    clientIdAndQos.first,
                    clientIdAndQos.second
                );
            }
        }
    }
};

void wup() {
    QlobberSub matcher;
    matcher.add("foo.bar", {
        "test1", "foo.bar", at_least_once
    });
    matcher.remove("foo.bar", std::optional<Sub>({
        "test1", std::nullopt, std::nullopt
    }));
    matcher.match(std::vector<SubResult>(), "foo.bar", std::nullopt);
    matcher.test("foo.bar", {
        "test1", "foo.bar", std::nullopt
    });
}
*/

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    return QlobberSub::Initialize(env, exports);
}

NODE_API_MODULE(qlobber, Initialize);
