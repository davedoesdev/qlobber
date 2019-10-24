#include <vector>
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
    std::optional<QoS> qos;
};

struct SubStorage {
    SubStorage(const Sub& sub) :
        topic(sub.topic) {
        clientMap.insert_or_assign(sub.clientId, sub.qos.value());
    }
    std::string topic;
    std::unordered_map<std::string, QoS> clientMap;
};

class QlobberSub : public QlobberBase<Sub, SubStorage> {
private:
    std::size_t subscriptionsCount = 0;

    void initial_value_inserted(const Sub& sub) override {
        ++subscriptionsCount;
    }

    void add_value(SubStorage& existing, const Sub& sub) {
        if (existing.clientMap.insert_or_assign(sub.clientId, sub.qos.value()).second) {
            ++subscriptionsCount;
        }
    }

    bool remove_value(SubStorage& vals, const Sub& sub) {
        if (vals.clientMap.erase(sub.clientId) > 0) {
            --subscriptionsCount;
        }
        return vals.clientMap.empty();
    }
};

void wup() {
    QlobberSub matcher;
    matcher.add("foo.bar", { "test1", "foo.bar", at_least_once });
    matcher.remove("foo.bar", { "test1", "foo.bar", std::nullopt });
}
