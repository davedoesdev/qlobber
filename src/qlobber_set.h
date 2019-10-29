#include "qlobber_set_base.h"
#include "qlobber_js_base.h"

template<typename Value, typename JSValue>
class QlobberSet :
    public QlobberJSBase<Value, JSValue, Napi::Object, QlobberSetBase>,
    public Napi::ObjectWrap<QlobberSet<Value, JSValue>> {
public:
    QlobberSet(const Napi::CallbackInfo& info) :
        QlobberJSBase<Value, JSValue, Napi::Object, QlobberSetBase>(info),
        Napi::ObjectWrap<QlobberSet<Value, JSValue>>(info) {}

private:
    Napi::Object NewMatchResult(const Napi::Env& env) override {
        return env.Global().Get("Map").As<Napi::Function>().New({});
    }

    void add_values(Napi::Object& dest,
                    const SetStorage<Value>& origin,
                    const std::nullptr_t&) override {
        const auto env = dest.Env();
        const auto Map = env.Global().Get("Map").As<Napi::Function>();
        const auto proto = Map.Get("prototype").As<Napi::Object>();
        const auto add = proto.Get("add").As<Napi::Function>();
        for (const auto& v : origin) {
            add.Call({ dest, JSValue::New(env, v) });
        }
    }
};
