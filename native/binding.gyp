{
  "targets": [
    {
      "target_name": "qlobber_native",
      "sources": [ "src/qlobber.cc", "src/rwlock.cc" ],
      "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],

      'cflags+': [ '-std=gnu++17', '-Wall', '-Wextra', '-Werror' ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions', '-std=gnu++1y', '-std=gnu++0x', '-std=gnu++14' ],
      'libraries': [ '-lboost_context' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'CLANG_CXX_LANGUAGE_STANDARD': 'c++17',
        'WARNING_CFLAGS': [ '-Wall', '-Wextra' ],
        'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}
