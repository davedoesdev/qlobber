#pragma once

#include <napi.h>
#include "options.h"

struct JSOptions : Options {
    JSOptions(const Napi::CallbackInfo& info) {
        if ((info.Length() > 0) && info[0].IsObject()) {
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
            if (options.Has("max_words")) {
                max_words = static_cast<uint32_t>(options.Get("max_words").As<Napi::Number>());
            }
            if (options.Has("max_wildcard_somes")) {
                max_wildcard_somes = static_cast<uint32_t>(options.Get("max_wildcard_somes").As<Napi::Number>());
            }
        }
    }

    static Napi::Value get(const Napi::Env& env, const Options& options) {
        Napi::Object r = Napi::Object::New(env);
        r.Set("separator", options.separator);
        r.Set("wildcard_one", options.wildcard_one);
        r.Set("wildcard_some", options.wildcard_some);
        r.Set("cache_adds", options.cache_adds);
        r.Set("max_words", options.max_words);
        r.Set("max_wildcard_somes", options.max_wildcard_somes);
        return r;
    }
};
