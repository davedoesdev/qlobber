#include <optional>
#include "qlobber_base.h"

enum QoS {
    at_most_once = 0,
    at_least_once = 1,
    exactly_once = 2
};

enum RetainHandling {
    send = 0,
    send_if_not_exist = 1,
    do_not_send = 2
};

struct Sub {
    std::string clientId;
    std::string topic;
    QoS qos;
    RetainHandling rh;
    bool rap;
    bool nl;
};

struct IterSub {
    std::string clientId;
    std::optional<std::string> topic;
    QoS qos;
    RetainHandling rh;
    bool rap;
    bool nl;
};

struct SubStorageNode {
    QoS qos;
    RetainHandling rh;
    bool rap;
    bool nl;
};

struct SubStorage {
    SubStorage(const Sub& sub) :
        topic(sub.topic) {
        clientMap.insert_or_assign(sub.clientId, SubStorageNode {
            sub.qos,
            sub.rh,
            sub.rap,
            sub.nl
        });
    }
    std::string topic;
    std::unordered_map<std::string, SubStorageNode> clientMap;
};

struct SubTest {
    std::string clientId;
    std::string topic;
};

template<typename Value,
         typename MatchResult,
         typename Context,
         typename RemoveValue,
         typename TestValue,
         typename IterValue>
class QlobberSubBase;

struct SubState {
    std::size_t subscriptionsCount = 0;
};

template<typename MatchResult, typename Context>
class QlobberSubBase<Sub, MatchResult, Context, std::string, SubTest, IterSub> :
    public QlobberBase<Sub,
                       SubStorage,
                       std::string,
                       MatchResult,
                       Context,
                       SubTest,
                       IterSub,
                       SubState> {
public:
    QlobberSubBase() {}

    QlobberSubBase(const Options& options) :
        QlobberBase<Sub,
                    SubStorage,
                    std::string,
                    MatchResult,
                    Context,
                    SubTest,
                    IterSub,
                    SubState>(options) {}

    QlobberSubBase(const OptionsOrState<typename QlobberSubBase::State>& options_or_state) :
        QlobberBase<Sub,
                    SubStorage,
                    std::string,
                    MatchResult,
                    Context,
                    SubTest,
                    IterSub,
                    SubState>(options_or_state) {}

protected:
    void clear() override {
        WriteLock(this->state->rwlock);
        this->state->subscriptionsCount = 0;
        QlobberBase<Sub,
                    SubStorage,
                    std::string,
                    MatchResult,
                    Context,
                    SubTest,
                    IterSub,
                    SubState>::clear_unlocked();
    }

private:
    void initial_value_inserted(const Sub& sub) override {
        ++this->state->subscriptionsCount;
    }

    void add_value(SubStorage& existing, const Sub& sub) override {
        if (existing.clientMap.insert_or_assign(sub.clientId, SubStorageNode {
            sub.qos,
            sub.rh,
            sub.rap,
            sub.nl
        }).second) {
            ++this->state->subscriptionsCount;
        }
    }

    bool remove_value(SubStorage& vals,
                      const std::optional<const std::string>& clientId) override {
        if (vals.clientMap.erase(clientId.value()) > 0) {
            --this->state->subscriptionsCount;
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
                     const std::optional<const std::string>& topic) override {
        if (!topic) {
            for (const auto& clientIdAndNode : storage.clientMap) {
                sink(IterSub {
                    clientIdAndNode.first,
                    std::optional<std::string>(storage.topic),
                    clientIdAndNode.second.qos,
                    clientIdAndNode.second.rh,
                    clientIdAndNode.second.rap,
                    clientIdAndNode.second.nl
                });
            }
        } else if (storage.topic == topic.value()) {
            for (const auto& clientIdAndNode : storage.clientMap) {
                sink(IterSub {
                    clientIdAndNode.first,
                    std::nullopt,
                    clientIdAndNode.second.qos,
                    clientIdAndNode.second.rh,
                    clientIdAndNode.second.rap,
                    clientIdAndNode.second.nl
                });
            }
        }
    }

    void visit_values(typename boost::coroutines2::coroutine<Visit<Sub>>::push_type& sink,
                      const SubStorage& storage) override {
        for (const auto& entry : storage.clientMap) {
            sink({
                Visit<Sub>::value,
                VisitData<Sub> {
                    std::variant<std::string, Sub>(
                        std::in_place_index<1>,
                        Sub {
                            entry.first,
                            storage.topic,
                            entry.second.qos,
                            entry.second.rh,
                            entry.second.rap,
                            entry.second.nl
                        })
                }
            });
        }
    }
};
