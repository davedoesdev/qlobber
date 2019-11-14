#include "qlobber_vec_base.h"
#include "qlobber_js_base.h"

template<typename Value, typename JSValue>
class QlobberVec :
    public QlobberJSBase<Value, JSValue, Napi::Array, QlobberVecBase>,
    public Napi::ObjectWrap<QlobberVec<Value, JSValue>> {
public:
    QlobberVec(const Napi::CallbackInfo& info) :
        QlobberJSBase<Value, JSValue, Napi::Array, QlobberVecBase>(info),
        Napi::ObjectWrap<QlobberVec<Value, JSValue>>(info) {}

private:
    Napi::Array NewMatchResult(const Napi::Env& env) override {
        return Napi::Array::New(env);
    }

    void add_values(Napi::Array& dest,
                    const VecStorage<Value>& origin,
                    const std::nullptr_t&) override {
        const auto env = dest.Env();
        for (const auto& v : origin) {
            dest.Set(dest.Length(), FromValue<Value, JSValue>(env, v));
        }
    }

    Napi::Object get_object() override {
        return Napi::ObjectWrap<QlobberVec<Value, JSValue>>::Value();
    }
};
