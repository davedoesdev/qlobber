#include <optional>
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
    QlobberSubBase() {}

    QlobberSubBase(const Options& options) :
        QlobberBase<Sub, SubStorage, std::string, MatchResult, Context, SubTest>(options) {}

    // Returns whether client is last subscriber to topic
    bool test_values(const SubStorage& existing,
                     const SubTest& val) override {
        return (existing.topic == val.topic) &&
               (existing.clientMap.size() == 1) &&
               (existing.clientMap.count(val.clientId) == 1);
    }

    void clear() override {
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

/*
#include <vector>
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
