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

struct IterSub {
    std::string clientId;
    std::optional<std::string> topic;
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

struct SubTest {
    std::string clientId;
    std::string topic;
};

template<typename MatchResult, typename Context, typename IterSub>
class QlobberSubBase :
    public QlobberBase<Sub,
                       SubStorage,
                       std::string,
                       MatchResult,
                       Context,
                       SubTest,
                       IterSub> {
public:
    QlobberSubBase() {}

    QlobberSubBase(const Options& options) :
        QlobberBase<Sub,
                    SubStorage,
                    std::string,
                    MatchResult,
                    Context,
                    SubTest,
                    IterSub>(options) {}

protected:
    void clear() override {
        subscriptionsCount = 0;
        QlobberBase<Sub,
                    SubStorage,
                    std::string,
                    MatchResult,
                    Context,
                    SubTest,
                    IterSub>::clear();
    }

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

    // Returns whether client is last subscriber to topic
    bool test_values(const SubStorage& existing,
                     const SubTest& val) override {
        return (existing.topic == val.topic) &&
               (existing.clientMap.size() == 1) &&
               (existing.clientMap.count(val.clientId) == 1);
    }

    void iter_values(typename boost::coroutines2::coroutine<IterSub>::push_type& sink,
                     const SubStorage& storage,
                     const std::optional<const std::string>& topic) {
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

    void visit_values(typename boost::coroutines2::coroutine<Visit<Sub>>::push_type& sink,
                      const SubStorage& storage) {
        for (const auto& entry : storage.clientMap) {
            sink({
                Visit<Sub>::value,
                VisitData<Sub> {
                    std::variant<std::string, Sub>(
                        std::in_place_index<1>,
                        Sub {
                            entry.first,
                            storage.topic,
                            entry.second
                        })
                }
            });
        }
    }
};
