#include <napi.h>
#include "qlobber_sub.h"
#include "qlobber_vec.h"

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    Initialize<QlobberSub>(env, "QlobberSub", exports);
    Initialize<QlobberVec<std::string, Napi::String>>(env, "QlobberString", exports);
    return exports;
}

NODE_API_MODULE(qlobber, Initialize);
