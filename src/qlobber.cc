#include <napi.h>
#include "qlobber_sub.h"
#include "qlobber_vec.h"
#include "qlobber_set.h"
#include "qlobber_true.h"

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    Initialize<QlobberVec<std::string, Napi::String>>(env, "QlobberString", exports);
    Initialize<QlobberVec<double, Napi::Number>>(env, "QlobberNumber", exports);
    Initialize<QlobberSet<std::string, Napi::String>>(env, "QlobberDedupString", exports);
    Initialize<QlobberSet<double, Napi::Number>>(env, "QlobberDedupNumber", exports);
    Initialize<QlobberTrue>(env, "QlobberTrue", exports);
    Initialize<QlobberSub>(env, "QlobberSub", exports);
    return exports;
}

NODE_API_MODULE(qlobber, Initialize);
