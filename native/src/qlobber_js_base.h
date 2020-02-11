#pragma once

#include <vector>
#include "js_options.h"

template<typename T>
Napi::Value MatchResultValue(const Napi::Env&, T& r) {
    return r;
}

template<typename Value, typename JSValue>
JSValue FromValue(const Napi::Env& env, const Value& v) {
    return JSValue::New(env, v);
}

template<typename Value, typename JSValue>
Value ToValue(const JSValue& v) {
    return v;
}

Napi::Function GetCallback(const Napi::CallbackInfo& info) {
    const auto len = info.Length();
    if (len > 0) {
        const auto cb = info[len - 1];
        if (cb.IsFunction()) {
            return cb.As<Napi::Function>();
        }
    }
    return Napi::Function::New(info.Env(), [](const Napi::CallbackInfo&) {});
}

template<typename Value,
         typename JSValue,
         typename MatchResult,
         typename Context,
         template<typename, typename, typename, typename, typename, typename> typename Base,
         typename RemoveValue = Value,
         typename TestValue = Value,
         typename IterValue = Value>
class QlobberJSCommon :
    public Base<Value, MatchResult, Context, RemoveValue, TestValue, IterValue> {
public:
    QlobberJSCommon(const Napi::CallbackInfo& info) :
        Base<Value, MatchResult, Context, RemoveValue, TestValue, IterValue>(
            JSOptionsOrState<typename QlobberJSCommon::State>(info)) {}

    Napi::Value Add(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        try {
            this->add(topic, get_add_value(info));
        } catch (std::exception& e) {
            throw Napi::Error::New(info.Env(), e.what());
        }
        return info.This();
    }
    
    void AddAsync(const Napi::CallbackInfo& info) {
        (new AddAsyncWorker(this, info))->Queue();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        const auto topic = info[0].As<Napi::String>();
        try {
            this->remove(topic, get_remove_value(info, 2));
        } catch (std::exception& e) {
            throw Napi::Error::New(info.Env(), e.what());
        }
        return info.This();
    }

    void RemoveAsync(const Napi::CallbackInfo& info) {
        (new RemoveAsyncWorker(this, info))->Queue();
    }

    Napi::Value Match(const Napi::CallbackInfo& info) {
        const auto env = info.Env();
        const auto topic = info[0].As<Napi::String>();
        auto r = this->NewMatchResult(env);
        try {
            this->match(r, topic, this->get_context(info));
        } catch (std::exception& e) {
            throw Napi::Error::New(env, e.what());
        }
        return MatchResultValue(env, r);
    }

    void MatchAsync(const Napi::CallbackInfo& info) {
        (new MatchAsyncWorker<decltype(
            this->match(info[0].As<Napi::String>(),
                        this->get_context(info)))>(this, info))->Queue();
    }

    Napi::Value Test(const Napi::CallbackInfo& info) {
        const auto env = info.Env();
        const auto topic = info[0].As<Napi::String>();
        try {
            return Napi::Boolean::New(env, this->test(topic, get_test_value(info)));
        } catch (std::exception& e) {
            throw Napi::Error::New(env, e.what());
        }
    }

    void TestAsync(const Napi::CallbackInfo& info) {
        (new TestAsyncWorker(this, info))->Queue();
    }

    Napi::Value Clear(const Napi::CallbackInfo& info) {
        this->clear();
        return info.This();
    }

    void ClearAsync(const Napi::CallbackInfo& info) {
        (new ClearAsyncWorker(this, info))->Queue();
    }

    Napi::Value GetOptions(const Napi::CallbackInfo& info) {
        return JSOptions::get(info.Env(), this->state->options);
    }
#ifdef DEBUG
    Napi::Value GetCounters(const Napi::CallbackInfo& info) {
        auto r = Napi::Object::New(info.Env());
        r.Set("add", this->state->counters.add);
        r.Set("remove", this->state->counters.remove);
        r.Set("match", this->state->counters.match);
        r.Set("match_some", this->state->counters.match_some);
        r.Set("match_iter", this->state->counters.match_iter);
        r.Set("match_some_iter", this->state->counters.match_some_iter);
        r.Set("test", this->state->counters.test);
        r.Set("test_some", this->state->counters.test_some);
        return r;
    }

    void ResetCounters(const Napi::CallbackInfo& info) {
        this->state->counters = { 0, 0, 0, 0, 0, 0, 0, 0 };
    }

    Napi::Value GetRefCount(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), this->state->ref_count);
    }
#endif
    Napi::Value GetVisitor(const Napi::CallbackInfo& info) {
        return Napi::External<Visitor>::New(
            info.Env(),
            new Visitor(this->visit()),
            [](Napi::Env, Visitor* visitor) {
                delete visitor;
            });
    }

    void GetVisitorAsync(const Napi::CallbackInfo& info) {
        (new GetVisitorAsyncWorker(this, info))->Queue();
    }

    Napi::Value VisitNext(const Napi::CallbackInfo& info) {
        const auto visitor = info[0].As<Napi::External<Visitor>>().Data();

        if (visitor->it == visitor->it_end) {
            return info.Env().Undefined();
        }

        const auto r = VisitNext(info.Env(), visitor);
        ++visitor->it;
        return r;
    }

    void VisitNextAsync(const Napi::CallbackInfo& info) {
        (new VisitNextAsyncWorker(this, info))->Queue();
    }

    Napi::Value GetRestorer(const Napi::CallbackInfo& info) {
        return Napi::External<Restorer>::New(
            info.Env(),
            new Restorer(this->restore(GetRestorerOptions(info))),
            [](Napi::Env, Restorer* restorer) {
                delete restorer;
            });
    }

    void GetRestorerAsync(const Napi::CallbackInfo& info) {
        (new GetRestorerAsyncWorker(this, info))->Queue();
    }

    Napi::Value RestoreNext(const Napi::CallbackInfo& info) {
        const auto restorer = info[0].As<Napi::External<Restorer>>().Data();
        const auto obj = info[1].As<Napi::Object>();
        const auto v = RestoreNext(obj);
        if (v) {
            restorer->sink(*v);
        }
        return info.Env().Undefined();
    }

    void RestoreNextAsync(const Napi::CallbackInfo& info) {
        (new RestoreNextAsyncWorker(this, info))->Queue();
    }

    Napi::Value MatchIter(const Napi::CallbackInfo& info) {
        const auto env = info.Env();
        try {
            return Napi::External<Iterator>::New(
                env,
                new Iterator(this->match_iter(info[0].As<Napi::String>(),
                                              this->get_context(info))),
                [](Napi::Env, Iterator* iterator) {
                    delete iterator;
                });
        } catch (std::exception& e) {
            throw Napi::Error::New(env, e.what());
        }
    }

    void MatchIterAsync(const Napi::CallbackInfo& info) {
        (new MatchIterAsyncWorker(this, info))->Queue();
    }

    Napi::Value MatchNext(const Napi::CallbackInfo& info) {
        const auto iterator = info[0].As<Napi::External<Iterator>>().Data();
        const auto env = info.Env();
        if (iterator->it == iterator->it_end) {
            return env.Undefined();
        }
        const auto r = MatchNext(env, iterator);
        ++iterator->it;
        return r;
    }

    void MatchNextAsync(const Napi::CallbackInfo& info) {
        (new MatchNextAsyncWorker(this, info))->Queue();
    }

    // For testing. Note this doesn't return shortcuts to ValueStorages but
    // instead to MatchResults. This is good enough for testing container
    // (vector and set) shortcuts.
    Napi::Value GetShortcuts(const Napi::CallbackInfo& info) {
        const auto env = info.Env();
        const auto Map = env.Global().Get("Map").As<Napi::Function>();
        const auto proto = Map.Get("prototype").As<Napi::Object>();
        const auto set = proto.Get("set").As<Napi::Function>();
        const auto ctx = this->get_context(info);
        Napi::Object r = Map.New({});

        for (const auto& topic_and_values : this->state->shortcuts) {
            auto entry = this->NewMatchResult(env);
            this->add_values(entry, topic_and_values.second, ctx);
            set.Call(r, {
                Napi::String::New(env, topic_and_values.first),
                MatchResultValue(env, entry)
            });
        }

        return r;
    }

    Napi::Value AddRef(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), this->add_ref());
    }

    Napi::Value Release(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), this->release());
    }

    Napi::Value GetStateAddress(const Napi::CallbackInfo& info) {
        typedef typename QlobberJSCommon::State *StatePtr;
        auto r = Napi::ArrayBuffer::New(info.Env(), sizeof(StatePtr));
        *static_cast<StatePtr*>(r.Data()) = this->state;
        return r;
    }

    Napi::Value GetUid(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), this->uid);
    }

    static Napi::Value GetDeletions(const Napi::CallbackInfo& info) {
        const auto env = info.Env();
        const auto Set = env.Global().Get("Set").As<Napi::Function>();
        const auto proto = Set.Get("prototype").As<Napi::Object>();
        const auto add = proto.Get("add").As<Napi::Function>();
        const auto r = Set.New({});
        for (const auto& uid : QlobberJSCommon::deletions) {
            add.Call(r, { Napi::Number::New(env, uid) });
        }
        return r;
    }

protected:
    virtual typename std::remove_const<Context>::type get_context(const Napi::CallbackInfo& info) = 0;

    virtual MatchResult NewMatchResult(const Napi::Env& env) = 0;

    virtual Napi::Object get_object() = 0;

    virtual Value get_add_value(const Napi::CallbackInfo& info) = 0;

    virtual std::optional<const RemoveValue> get_remove_value(const Napi::Value& v) = 0;

    virtual TestValue get_test_value(const Napi::CallbackInfo& info) = 0;

private:
    std::optional<const RemoveValue> get_remove_value(const Napi::CallbackInfo& info, const size_t n) {
        if (info.Length() < n) {
            return std::nullopt;
        }
        const auto v = info[1];
        if (v.IsUndefined() || v.IsNull()) {
            return std::nullopt;
        }
        return get_remove_value(v);
    }

    template<typename T>
    struct Puller {
        typedef typename boost::coroutines2::coroutine<T> coro_t;
        typedef typename coro_t::pull_type pull_t;
        Puller(pull_t&& source) :
            source(std::move(source)),
            it(begin(this->source)),
            it_end(end(this->source)) {}
        pull_t source;
        typename pull_t::iterator it, it_end;
    };

    using Visitor = Puller<Visit<Value>>;
    using Iterator = Puller<IterValue>;

    Napi::Object VisitNext(const Napi::Env& env, const Visitor* visitor) {
        Napi::Object r = Napi::Object::New(env);

        switch (visitor->it->type) {
            case Visit<Value>::start_entries:
                r.Set("type", "start_entries");
                break;

            case Visit<Value>::entry:
                r.Set("type", "entry");
                r.Set("key", Napi::String::New(
                    env, std::get<0>(*visitor->it->v)));
                break;

            case Visit<Value>::end_entries:
                r.Set("type", "end_entries");
                break;

            case Visit<Value>::start_values:
                r.Set("type", "start_values");
                break;

            case Visit<Value>::value:
                r.Set("type", "value");
                r.Set("value", FromValue<Value, JSValue>(
                    env, std::get<1>(*visitor->it->v)));
                break;

            case Visit<Value>::end_values:
                r.Set("type", "end_values");
                break;
        }

        return r;
    }

    std::unique_ptr<Visit<Value>> RestoreNext(const Napi::Object& obj) {
        const std::string type = obj.Get("type").As<Napi::String>();

        if (type == "start_entries") {
            return std::make_unique<Visit<Value>>(
                Visit<Value>::start_entries,
                std::nullopt
            );
        }
        
        if (type == "entry") {
            return std::make_unique<Visit<Value>>(
                Visit<Value>::entry, 
                VisitData<Value> {
                    std::variant<std::string, Value>(
                        std::in_place_index<0>,
                        obj.Get("key").As<Napi::String>())
                }
            );
        }

        if (type == "end_entries") {
            return std::make_unique<Visit<Value>>(
                Visit<Value>::end_entries,
                std::nullopt
            );
        }
        
        if (type == "start_values") {
            return std::make_unique<Visit<Value>>(
                Visit<Value>::start_values,
                std::nullopt
            );
        }

        if (type == "value") {
            return std::make_unique<Visit<Value>>(
                Visit<Value>::value,
                VisitData<Value> {
                    std::variant<std::string, Value>(
                        std::in_place_index<1>,
                        ToValue<Value, JSValue>(obj.Get("value").As<JSValue>()))
                }
            );
        }

        if (type == "end_values") {
            return std::make_unique<Visit<Value>>(
                Visit<Value>::end_values, std::nullopt
            );
        }

        return nullptr;
    }

    Napi::Object MatchNext(const Napi::Env& env, const Iterator* iterator) {
        Napi::Object r = Napi::Object::New(env);
        r.Set("value", FromValue<IterValue, JSValue>(env, *iterator->it));
        return r;
    }

    bool GetRestorerOptions(const Napi::CallbackInfo& info) {
        if ((info.Length() > 0) && info[0].IsObject()) {
            auto options = info[0].As<Napi::Object>();
            if (options.Has("cache_adds")) {
                return options.Get("cache_adds").As<Napi::Boolean>();
            }
        }
        return false;
    }

    struct Restorer {
        typedef typename boost::coroutines2::coroutine<Visit<Value>> coro_visit_t;
        typedef typename coro_visit_t::push_type push_t;
        Restorer(push_t sink) :
            sink(std::move(sink)) {}
        push_t sink;
    };

    class QlobberAsyncWorker : public Napi::AsyncWorker {
    public:
        QlobberAsyncWorker(QlobberJSCommon* qlobber,
                           const Napi::CallbackInfo& info) :
            Napi::AsyncWorker(GetCallback(info)),
            qlobber(qlobber),
            qlobber_ref(Napi::Persistent(qlobber->get_object())) {}

    protected:
        QlobberJSCommon* qlobber;
        Napi::ObjectReference qlobber_ref;
    };

    class TopicAsyncWorker : public QlobberAsyncWorker {
    public:
        TopicAsyncWorker(QlobberJSCommon* qlobber,
                         const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info),
            topic(info[0].As<Napi::String>()) {}

    protected:
        std::string topic;
    };

    class AddAsyncWorker : public TopicAsyncWorker {
    public:
        AddAsyncWorker(QlobberJSCommon* qlobber,
                       const Napi::CallbackInfo& info) :
            TopicAsyncWorker(qlobber, info),
            value(qlobber->get_add_value(info)) {}

        void Execute() override {
            this->qlobber->add(this->topic, value);
        }

    private:
        Value value;
    };

    class RemoveAsyncWorker : public TopicAsyncWorker {
    public:
        RemoveAsyncWorker(QlobberJSCommon* qlobber,
                          const Napi::CallbackInfo& info) :
            TopicAsyncWorker(qlobber, info),
            value(qlobber->get_remove_value(info, 3)) {}

        void Execute() override {
            this->qlobber->remove(this->topic, value);
        }

    private:
        std::optional<const RemoveValue> value;
    };

    template<typename Storages>
    class MatchAsyncWorker : public TopicAsyncWorker {
    public:
        MatchAsyncWorker(QlobberJSCommon* qlobber,
                         const Napi::CallbackInfo& info) :
            TopicAsyncWorker(qlobber, info),
            context(qlobber->get_context(info)) {}

        void Execute() override {
            storages = this->qlobber->match(this->topic, this->context);
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            auto r = this->qlobber->NewMatchResult(env);
            for (const auto& s : storages) {
                this->qlobber->add_values(r, s, this->context);
            }
            return { env.Null(), MatchResultValue(env, r) };
        }

    private:
        typename std::remove_const<Context>::type context;
        Storages storages;
    };

    class TestAsyncWorker : public TopicAsyncWorker {
    public:
        TestAsyncWorker(QlobberJSCommon* qlobber,
                        const Napi::CallbackInfo& info) :
            TopicAsyncWorker(qlobber, info),
            value(qlobber->get_test_value(info)) {}

        void Execute() override {
            result = this->qlobber->test(this->topic, value);
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            return { env.Null(), Napi::Boolean::New(env, result) };
        }

    private:
        TestValue value;
        bool result;
    };

    class ClearAsyncWorker : public QlobberAsyncWorker {
    public:
        ClearAsyncWorker(QlobberJSCommon* qlobber,
                         const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info) {}

        void Execute() override {
            this->qlobber->clear();
        }
    };

    struct AsyncVisitor : Visitor {
        AsyncVisitor(typename Visitor::pull_t&& source) :
            Visitor(std::forward<typename Visitor::pull_t>(source)) {}
        bool first = true;
    };

    class GetVisitorAsyncWorker : public QlobberAsyncWorker {
    public:
        GetVisitorAsyncWorker(QlobberJSCommon* qlobber,
                              const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info) {}

        void Execute() override {
            visitor = new AsyncVisitor(this->qlobber->visit());
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            return {
                env.Null(), 
                Napi::External<AsyncVisitor>::New(
                    env,
                    visitor,
                    [](Napi::Env, AsyncVisitor* visitor) {
                        delete visitor;
                    })
            };
        }

        void OnError(const Napi::Error& e) override {
            delete visitor;
            QlobberAsyncWorker::OnError(e);
        }

    private:
        AsyncVisitor* visitor;
    };

    class VisitNextAsyncWorker : public QlobberAsyncWorker {
    public:
        VisitNextAsyncWorker(QlobberJSCommon* qlobber,
                             const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info),
            visitor(info[0].As<Napi::External<AsyncVisitor>>().Data()) {}

        void Execute() override {
            if ((visitor->it != visitor->it_end) && !visitor->first) {
                ++visitor->it;
            }
            visitor->first = false;
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            if (visitor->it == visitor->it_end) {
                return {};
            }
            return { env.Null(), this->qlobber->VisitNext(env, visitor) };
        }

    private:
        AsyncVisitor* visitor;
    };

    class GetRestorerAsyncWorker : public QlobberAsyncWorker {
    public:
        GetRestorerAsyncWorker(QlobberJSCommon* qlobber,
                               const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info),
            cache_adds(qlobber->GetRestorerOptions(info)) {}

        void Execute() override {
            restorer = new Restorer(this->qlobber->restore(cache_adds));
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            return {
                env.Null(), 
                Napi::External<Restorer>::New(
                    env,
                    restorer,
                    [](Napi::Env, Restorer* restorer) {
                        delete restorer;
                    })
            };
        }

        void OnError(const Napi::Error& e) override {
            delete restorer;
            QlobberAsyncWorker::OnError(e);
        }

    private:
        Restorer* restorer;
        bool cache_adds;
    };

    class RestoreNextAsyncWorker : public QlobberAsyncWorker {
    public:
        RestoreNextAsyncWorker(QlobberJSCommon* qlobber,
                               const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info),
            restorer(info[0].As<Napi::External<Restorer>>().Data()),
            v(this->qlobber->RestoreNext(info[1].As<Napi::Object>())) {}

        void Execute() override {
            if (v) {
                restorer->sink(*v);
            }
        }

    private:
        Restorer* restorer;
        std::unique_ptr<Visit<Value>> v;
    };

    struct AsyncIterator : Iterator {
        AsyncIterator(typename Iterator::pull_t&& source) :
            Iterator(std::forward<typename Iterator::pull_t>(source)) {}
        bool first = true;
    };

    class MatchIterAsyncWorker : public TopicAsyncWorker {
    public:
        MatchIterAsyncWorker(QlobberJSCommon* qlobber,
                             const Napi::CallbackInfo& info) :
            TopicAsyncWorker(qlobber, info),
            context(qlobber->get_context(info)) {}

        void Execute() override {
            iterator = new AsyncIterator(
                this->qlobber->match_iter(this->topic, this->context));
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            return {
                env.Null(), 
                Napi::External<AsyncIterator>::New(
                    env,
                    iterator,
                    [](Napi::Env, AsyncIterator* iterator) {
                        delete iterator;
                    })
            };
        }

        void OnError(const Napi::Error& e) override {
            delete iterator;
            TopicAsyncWorker::OnError(e);
        }

    private:
        AsyncIterator* iterator;
        typename std::remove_const<Context>::type context;
    };

    class MatchNextAsyncWorker : public QlobberAsyncWorker {
    public:
        MatchNextAsyncWorker(QlobberJSCommon* qlobber,
                             const Napi::CallbackInfo& info) :
            QlobberAsyncWorker(qlobber, info),
            iterator(info[0].As<Napi::External<AsyncIterator>>().Data()) {}

        void Execute() override {
            if ((iterator->it != iterator->it_end) && !iterator->first) {
                ++iterator->it;
            }
            iterator->first = false;
        }

        std::vector<napi_value> GetResult(Napi::Env env) override {
            if (iterator->it == iterator->it_end) {
                return {};
            }
            return { env.Null(), this->qlobber->MatchNext(env, iterator) };
        }

    private:
        AsyncIterator* iterator;
        bool ended;
    };
};

template<typename Value,
         typename JSValue,
         typename MatchResult,
         template<typename, typename, typename, typename, typename, typename> typename Base>
class QlobberJSBase :
    public QlobberJSCommon<Value, JSValue, MatchResult, const std::nullptr_t, Base> {
public:
    QlobberJSBase(const Napi::CallbackInfo& info) :
        QlobberJSCommon<Value, JSValue, MatchResult, const std::nullptr_t, Base>(info) {}
                        
    virtual ~QlobberJSBase() {}

private:
    std::nullptr_t get_context(const Napi::CallbackInfo& info) override {
        return nullptr;
    }

    Value get_add_value(const Napi::CallbackInfo& info) override {
        return info[1].As<JSValue>();
    }

    std::optional<const Value> get_remove_value(const Napi::Value& v) override {
        return v.As<JSValue>();
    }

    Value get_test_value(const Napi::CallbackInfo& info) override {
        return info[1].As<JSValue>();
    }
};

template<typename T>
std::vector<Napi::ClassPropertyDescriptor<T>> Properties() {
    return {};
}

template<typename T>
void Initialize(Napi::Env env, const char* name, Napi::Object exports) {
    std::vector<Napi::ClassPropertyDescriptor<T>> props {
        T::InstanceMethod("add", &T::Add),
        T::InstanceMethod("add_async", &T::AddAsync),
        T::InstanceMethod("remove", &T::Remove),
        T::InstanceMethod("remove_async", &T::RemoveAsync),
        T::InstanceMethod("match", &T::Match),
        T::InstanceMethod("match_async", &T::MatchAsync),
        T::InstanceMethod("test", &T::Test),
        T::InstanceMethod("test_async", &T::TestAsync),
        T::InstanceMethod("clear", &T::Clear),
        T::InstanceMethod("clear_async", &T::ClearAsync),
        T::InstanceMethod("get_visitor", &T::GetVisitor),
        T::InstanceMethod("get_visitor_async", &T::GetVisitorAsync),
        T::InstanceMethod("visit_next", &T::VisitNext),
        T::InstanceMethod("visit_next_async", &T::VisitNextAsync),
        T::InstanceMethod("get_restorer", &T::GetRestorer),
        T::InstanceMethod("get_restorer_async", &T::GetRestorerAsync),
        T::InstanceMethod("restore_next", &T::RestoreNext),
        T::InstanceMethod("restore_next_async", &T::RestoreNextAsync),
        T::InstanceMethod("match_iter", &T::MatchIter),
        T::InstanceMethod("match_iter_async", &T::MatchIterAsync),
        T::InstanceMethod("match_next", &T::MatchNext),
        T::InstanceMethod("match_next_async", &T::MatchNextAsync),
        T::InstanceMethod("add_ref", &T::AddRef),
        T::InstanceMethod("release", &T::Release),
        T::InstanceAccessor("options", &T::GetOptions, nullptr),
        T::InstanceAccessor("_shortcuts", &T::GetShortcuts, nullptr),
        T::InstanceAccessor("state_address", &T::GetStateAddress, nullptr),
#ifdef DEBUG
        T::InstanceAccessor("_counters", &T::GetCounters, nullptr),
        T::InstanceMethod("_reset_counters", &T::ResetCounters),
        T::InstanceAccessor("_ref_count", &T::GetRefCount, nullptr),
        T::InstanceAccessor("_uid", &T::GetUid, nullptr),
        T::StaticAccessor("_deletions", &T::GetDeletions, nullptr),
#endif
    };

    const auto props2 = Properties<T>();
    props.insert(props.end(), props2.begin(), props2.end());

    exports.Set(name, T::DefineClass(env, name, props));
}
