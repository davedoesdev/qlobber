#include <napi.h>
#include "qlobber_sub.h"
#include "qlobber_vec.h"

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    QlobberSub::Initialize(env, exports);
    Initialize<QlobberVec<std::string, Napi::String>>(env, "QlobberNative", exports);
    return exports;
}

NODE_API_MODULE(qlobber, Initialize);
