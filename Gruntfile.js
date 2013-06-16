/*jslint node: true */
"use strict";

module.exports = function (grunt)
{
    grunt.initConfig(
    {
        jslint: {
            files: [ 'Gruntfile.js', 'index.js', 'test/*.js' ],
            directives: {
                white: true
            }
        },

        cafemocha: {
            src: 'test/*.js'
        },

        apidox: {
            input: 'lib/qlobber.js',
            output: 'README.md',
            fullSourceDescription: true
        }
    });
    
    grunt.loadNpmTasks('grunt-jslint');
    grunt.loadNpmTasks('grunt-cafe-mocha');
    grunt.loadNpmTasks('grunt-apidox');

    grunt.registerTask('lint', 'jslint');
    grunt.registerTask('test', 'cafemocha');
    grunt.registerTask('docs', 'apidox');
    grunt.registerTask('default', ['jslint', 'cafemocha']);
};
