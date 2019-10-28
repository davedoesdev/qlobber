#include <napi.h>
#include "qlobber_sub.h"

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    return QlobberSub::Initialize(env, exports);
}

NODE_API_MODULE(qlobber, Initialize);
