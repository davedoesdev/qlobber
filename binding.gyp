{
  "targets": [
    {
      "target_name": "qlobber",
      "sources": [ "src/qlobber_sub.cc" ],
      "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],

      'cflags+': [ '-std=gnu++17' ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions', '-std=gnu++1y' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      'conditions': [
        [
          'coverage == "true"',
          {
            'cflags+': [ '--coverage' ],
            'ldflags+': [ '--coverage' ]
          }
        ]
      ]
    }
  ]
}
