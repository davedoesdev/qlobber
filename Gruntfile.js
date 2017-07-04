/*jslint node: true */
"use strict";

module.exports = function (grunt)
{
    grunt.initConfig(
    {
        jshint: {
            all: [ 'Gruntfile.js', 'index.js', 'lib/*.js', 'test/*.js', 'bench/**/*.js' ],
            options: {
                esversion: 6
            }
        },

        mochaTest: {
            src: 'test/*.js'
        },

        apidox: {
            input: 'lib/qlobber.js',
            output: 'README.md',
            fullSourceDescription: true,
            extraHeadingLevels: 1
        },

        shell: {
            cover: {
                command: "./node_modules/.bin/nyc -x Gruntfile.js -x 'test/**' ./node_modules/.bin/grunt test"
            },

            cover_report: {
                command: './node_modules/.bin/nyc report -r lcov'
            },

            cover_check: {
                command: './node_modules/.bin/nyc check-coverage --statements 100 --branches 100 --functions 100 --lines 100'
            },

            coveralls: {
                command: 'cat coverage/lcov.info | coveralls'
            },

            bench: {
                command: './node_modules/.bin/bench -c 20000 -i bench/options/default.js,bench/options/dedup.js -k options bench/add_match_remove bench/match'
            },

            'bench-check': {
                command: './node_modules/.bin/bench -c 20000 -i bench/options/check.js,bench/options/check-dedup.js -k options bench/add_match_remove bench/match'
            },

            'bench-add-many': {
                command: './node_modules/.bin/bench -c 1 -i bench/options/default.js,bench/options/dedup.js -k options bench/add_many.js'
            },

            'bench-match-many': {
                command: './node_modules/.bin/bench -c 1 -i bench/options/default.js,bench/options/dedup.js -k options bench/match_many.js'
            }
        }
    });
    
    grunt.loadNpmTasks('grunt-contrib-jshint');
    grunt.loadNpmTasks('grunt-mocha-test');
    grunt.loadNpmTasks('grunt-apidox');
    grunt.loadNpmTasks('grunt-shell');

    grunt.registerTask('lint', 'jshint');
    grunt.registerTask('test', 'mochaTest');
    grunt.registerTask('docs', 'apidox');
    grunt.registerTask('coverage', ['shell:cover',
                                    'shell:cover_report',
                                    'shell:cover_check']);
    grunt.registerTask('coveralls', 'shell:coveralls');
    grunt.registerTask('bench', ['shell:bench',
                                 'shell:bench-add-many',
                                 'shell:bench-match-many']);
    grunt.registerTask('bench-check', 'shell:bench-check');
    grunt.registerTask('bench-add-many', 'shell:bench-add-many');
    grunt.registerTask('bench-match-many', 'shell:bench-match-many');
    grunt.registerTask('default', ['jshint', 'mochaTest']);
};
