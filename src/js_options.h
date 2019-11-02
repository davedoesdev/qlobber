#pragma once

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

    static Napi::Value get(const Napi::Env& env, const Options& options) {
        Napi::Object r = Napi::Object::New(env);
        r.Set("separator", options.separator);
        r.Set("wildcard_one", options.wildcard_one);
        r.Set("wildcard_some", options.wildcard_some);
        r.Set("cache_adds", options.cache_adds);
        return r;
    }
};
