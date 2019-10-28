#include <napi.h>
#include "options.h"

struct JSOptions : Options {
    JSOptions(const Napi::CallbackInfo& info) {
        if (info.Length() > 0) {
            auto options = info[0].As<Napi::Object>();
            if (options.Has("separator")) {
                separator = options.Get("separator").As<Napi::String>();
            }
            if (options.Has("wildcard_one")) {
                wildcard_one = options.Get("wildcard_one").As<Napi::String>();
            }
            if (options.Has("wildcard_some")) {
                wildcard_some = options.Get("wildcard_some").As<Napi::String>();
            }
            if (options.Has("cache_adds")) {
                cache_adds = options.Get("cache_adds").As<Napi::Boolean>();
            }
        }
    }
};
