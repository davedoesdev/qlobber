#include <vector>
#include "qlobber_base.h"

struct Sub {
    std::string clientId;
    std::string topic;
    uint8_t qos;
};

struct SubStorage {
    std::string topic;
    std::unordered_map<std::string, uint8_t> clientMap;
};

class QlobberSub : public QlobberBase<Sub, SubStorage> {
private:
    uint64_t subscriptionsCount = 0;

    SubStorage initial_value(const Sub& sub) {
        ++subscriptionsCount;
        SubStorage storage;
        storage.topic = sub.topic;
        storage.clientMap[sub.clientId] = sub.qos;
        return storage;
    }

    void add_value(SubStorage& existing, const Sub& sub) {
        auto size = existing.clientMap.size();
        existing.clientMap[sub.clientId] = sub.qos;
        if (existing.clientMap.size() > size) {
            ++subscriptionsCount;
        }
    }
};

void wup() {
    QlobberSub sub;    
    sub.add("foo.bar", { "test1", "foo.bar", 1 });
}
