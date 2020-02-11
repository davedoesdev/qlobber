#include "qlobber_set_base.h"
#include "qlobber_js_base.h"

template<typename Value, typename JSValue>
class QlobberSet :
    public QlobberJSBase<Value,
                         JSValue,
                         std::pair<Napi::Object, Napi::Function>,
                         QlobberSetBase>,
    public Napi::ObjectWrap<QlobberSet<Value, JSValue>> {
public:
    QlobberSet(const Napi::CallbackInfo& info) :
        QlobberJSBase<Value,
                      JSValue,
                      std::pair<Napi::Object, Napi::Function>,
                      QlobberSetBase>(info),
        Napi::ObjectWrap<QlobberSet<Value, JSValue>>(info) {}

private:
    std::pair<Napi::Object, Napi::Function>
    NewMatchResult(const Napi::Env& env) override {
        const auto Set = env.Global().Get("Set").As<Napi::Function>();
        const auto proto = Set.Get("prototype").As<Napi::Object>();
        const auto add = proto.Get("add").As<Napi::Function>();
        return std::make_pair(Set.New({}), add);
    }

    void add_values(std::pair<Napi::Object, Napi::Function>& r,
                    const SetStorage<Value>& origin,
                    const std::nullptr_t&) override {
        const auto env = r.first.Env();
        for (const auto& v : origin) {
            r.second.Call(r.first, { FromValue<Value, JSValue>(env, v) });
        }
    }

    Napi::Object get_object() override {
        return Napi::ObjectWrap<QlobberSet<Value, JSValue>>::Value();
    }
};

template<>
Napi::Value MatchResultValue<std::pair<Napi::Object, Napi::Function>>(
    const Napi::Env&,
    std::pair<Napi::Object, Napi::Function>& r) {
    return r.first;
}
