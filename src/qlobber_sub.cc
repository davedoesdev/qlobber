#include <vector>
#include "qlobber_base.h"

struct Sub {
    std::string clientId;
    std::string topic;
    uint8_t qos;
};

struct SubStorage {
    SubStorage(const Sub& sub) :
        topic(sub.topic) {
        clientMap.insert_or_assign(sub.clientId, sub.qos);
    }
    std::string topic;
    std::unordered_map<std::string, uint8_t> clientMap;
};

class QlobberSub : public QlobberBase<Sub, SubStorage> {
private:
    uint64_t subscriptionsCount = 0;

    void initial_value_inserted(const Sub&) override {
        ++subscriptionsCount;
    }

    void add_value(SubStorage& existing, const Sub& sub) {
        if (existing.clientMap.insert_or_assign(sub.clientId, sub.qos).second) {
            ++subscriptionsCount;
        }
    }
};

void wup() {
    QlobberSub sub;    
    sub.add("foo.bar", { "test1", "foo.bar", 1 });
}
