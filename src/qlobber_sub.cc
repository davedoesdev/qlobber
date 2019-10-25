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
    std::optional<std::string> topic;
    std::optional<QoS> qos;
};

struct SubStorage {
    SubStorage(const Sub& sub) :
        topic(sub.topic.value()) {
        clientMap.insert_or_assign(sub.clientId, sub.qos.value());
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

template<typename MatchResult, typename Context>
class QlobberSubBase :
    public QlobberBase<Sub, SubStorage, MatchResult, Context> {
public:
    // Returns whether client is last subscriber to topic
    bool test_values(const SubStorage& existing,
                     const Sub& val) override {
        return (existing.topic == val.topic.value()) &&
               (existing.clientMap.size() == 1) &&
               (existing.clientMap.count(val.clientId) == 1);
    }

private:
    std::size_t subscriptionsCount = 0;

    void initial_value_inserted(const Sub& sub) override {
        ++subscriptionsCount;
    }

    void add_value(SubStorage& existing, const Sub& sub) override {
        if (existing.clientMap.insert_or_assign(sub.clientId,
                                                sub.qos.value()).second) {
            ++subscriptionsCount;
        }
    }

    bool remove_value(SubStorage& vals,
                      const std::optional<const Sub>& sub) override {
        if (vals.clientMap.erase(sub.value().clientId) > 0) {
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

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
        exports.Set("QlobberSubNative", DefineClass(env, "QlobberSubNative", {




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
    matcher.match("foo.bar", std::nullopt);
    matcher.test("foo.bar", {
        "test1", "foo.bar", std::nullopt
    });
}
*/

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    return QlobberSub::Initialize(env, exports);
}

NODE_API_MODULE(qlobber_sub, Initialize);
