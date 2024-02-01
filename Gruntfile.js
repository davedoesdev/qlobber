/*jslint node: true */
"use strict";

const c8 = "npx c8 -x Gruntfile.js -x 'test/**'";

module.exports = function (grunt)
{
    grunt.initConfig(
    {
        jshint: {
            all: [ 'Gruntfile.js', 'index.js', 'lib/*.js', 'aedes/**/*.js', 'test/*.js', 'bench/**/*.js' ],
            options: {
                esversion: 11,
                node: true
            }
        },

        apidox: {
            input: 'lib/qlobber.js',
            output: 'README.md',
            fullSourceDescription: true,
            extraHeadingLevels: 1
        },

        exec: Object.fromEntries(Object.entries({
            test: {
                cmd: 'node --expose-gc ./node_modules/.bin/mocha test/*.js'
            },

            cover: {
                cmd: `${c8} npx grunt test`
            },

            cover_report: {
                cmd: `${c8} report -r lcov`
            },

            cover_check: {
                cmd: `${c8} check-coverage --statements 100 --branches 100 --functions 100 --lines 100`
            },

            bench: {
                cmd: 'npx bench -c 20000 -i bench/options/default.js,bench/options/dedup.js,bench/options/mapval.js,bench/options/default-native.js,bench/options/dedup-native.js,bench/options/default-cache-splits.js -k options bench/add bench/add_match_remove bench/match bench/match_search bench/test'
            },

            'bench-check': {
                cmd: 'npx bench -c 20000 -i bench/options/check-default.js,bench/options/check-dedup.js,bench/options/check-mapval.js,bench/options/check-default-native.js,bench/options/check-dedup-native.js -k options bench/add bench/add_match_remove bench/match bench/match_search bench/test'
            },

            'bench-many': {
                cmd: 'npx bench -c 1 -i bench/options/default.js,bench/options/dedup.js,bench/options/mapval.js,bench/options/default-native.js,bench/options/dedup-native.js,bench/options/default-cache-splits.js -k options bench/add_many bench/add_shortcut_many bench/match_many bench/match_search_many bench/test_many'
            }
        }).map(([k, v]) => [k, { stdio: 'inherit', ...v }]))
    });
    
    grunt.loadNpmTasks('grunt-contrib-jshint');
    grunt.loadNpmTasks('grunt-apidox');
    grunt.loadNpmTasks('grunt-exec');

    grunt.registerTask('lint', 'jshint');
    grunt.registerTask('test', 'exec:test');
    grunt.registerTask('docs', 'apidox');
    grunt.registerTask('coverage', ['exec:cover',
                                    'exec:cover_report',
                                    'exec:cover_check']);
    grunt.registerTask('bench', ['exec:bench',
                                 'exec:bench-many']);
    grunt.registerTask('bench-check', 'exec:bench-check');
    grunt.registerTask('default', ['lint', 'test']);
};
