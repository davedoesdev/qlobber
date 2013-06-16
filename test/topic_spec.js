/*globals rabbitmq_test_bindings : false,
          rabbitmq_bindings_to_remove : false,
          rabbitmq_expected_results_before_remove: false,
          rabbitmq_expected_results_after_remove : false,
          rabbitmq_expected_results_after_clear : false,
          describe: false,
          beforeEach: false,
          it: false */
/*jslint node: true */
"use strict";

var async = require('async'),
    expect = require('chai').expect,
    qlobber = require('..');

describe('qlobber', function ()
{
    var matcher;

    beforeEach(function (done)
    {
        matcher = new qlobber.Qlobber();
        done();
    });

    function add_bindings(bindings, in_series, mapper, done)
    {
        var each = in_series ? async.eachSeries : async.each;

        mapper = mapper || function (topic)
        {
            return topic;
        };

        each(bindings, function (topic_val, cb)
        {
            matcher.add(topic_val[0], mapper(topic_val[1]), cb);
        }, done);
    }

    it('should support adding bindings', function (done)
    {
        add_bindings(rabbitmq_test_bindings, true, null, function ()
        {
            expect(matcher.get_trie()).to.eql({"a":{"b":{"c":{".":["t1","t20"]},"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},".":["t11"],"#":{".":["t12"]}}},"#":{".":["t5"],"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{".":["t21"],"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]},"vodka":{"martini":{".":["t19"]}}});
            done();
        });
    });

    it('should pass rabbitmq test', function (done)
    {
        add_bindings(rabbitmq_test_bindings, false, null, function ()
        {
            async.each(rabbitmq_expected_results_before_remove, function (test, cb)
            {
                /*jslint unparam: true */
                matcher.match(test[0], function (err, vals)
                {
                    expect(vals, test[0]).to.eql(test[1].sort());
                    cb();
                });
            }, done);
        });
    });

    it('should support removing bindings', function (done)
    {
        add_bindings(rabbitmq_test_bindings, false, null, function ()
        {
            async.each(rabbitmq_bindings_to_remove, function (i, cb)
            {
                matcher.remove(rabbitmq_test_bindings[i-1][0],
                               rabbitmq_test_bindings[i-1][1],
                               cb);
            }, function ()
            {
                expect(matcher.get_trie()).to.eql({"a":{"b":{"c":{".":["t20"]},"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},"#":{".":["t12"]}}},"#":{"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]}});

                async.each(rabbitmq_expected_results_after_remove, function (test, cb)
                {
                    /*jslint unparam: true */
                    matcher.match(test[0], function (err, vals)
                    {
                        expect(vals, test[0]).to.eql(test[1].sort());
                        cb();
                    });
                }, function ()
                {
                    /*jslint unparam: true */
                    var remaining = rabbitmq_test_bindings.filter(
                    function (topic_val, i)
                    {
                        return rabbitmq_bindings_to_remove.indexOf(i + 1) < 0;
                    });
                    /*jslint unparam: false */

                    async.each(remaining, function (topic_val, cb)
                    {
                        matcher.remove(topic_val[0], topic_val[1], cb);
                    }, function ()
                    {
                        expect(matcher.get_trie()).to.eql({});

                        async.each(rabbitmq_expected_results_after_clear, function (test, cb)
                        {
                            /*jslint unparam: true */
                            matcher.match(test[0], function (err, vals)
                            {
                                expect(vals, test[0]).to.eql(test[1].sort());
                                cb();
                            });
                        }, done);
                    });
                });
            });
        });
    });

    it('should support clearing the bindings', function (done)
    {
        add_bindings(rabbitmq_test_bindings, false, null, function ()
        {
            matcher.clear(function ()
            {
                async.each(rabbitmq_expected_results_after_clear, function (test, cb)
                {
                    /*jslint unparam: true */
                    matcher.match(test[0], function (err, vals)
                    {
                        expect(vals, test[0]).to.eql(test[1].sort());
                        cb();
                    });
                }, done);
            });
        });
    });

    it('should support functions as values', function (done)
    {
        matcher = new qlobber.Qlobber(
        {
            compare: function (f1, f2)
            {
                return f1.topic < f2.topic ? -1 : f1.topic > f2.topic ? 1 : 0;
            }
        });

        add_bindings(rabbitmq_test_bindings, false,
        function (topic)
        {
            var f = function ()
            {
                return topic;
            };

            f.topic = topic;

            return f;
        },
        function ()
        {
            async.each(rabbitmq_expected_results_before_remove, function (test, cb)
            {
                /*jslint unparam: true */
                matcher.match(test[0], function (err, vals)
                {
                    expect(vals.map(function (f) { return f(); }), test[0])
                           .to.eql(test[1].sort());
                    cb();
                });
            }, done);
        });
    });

    it('should pass example in README', function (done)
    {
        matcher.add('foo.*', 'Q1', function ()
        {
            /*jslint unparam: true */
            matcher.match('foo.bar', function (err, vals)
            {
                expect(vals).to.eql(['Q1']);
                done();
            });
        });
    });

    it('should pass example in rabbitmq topic tutorial', function (done)
    {
        /*jslint unparam: true */
        async.parallel(
            [matcher.add.bind(matcher, '*.orange.*', 'Q1'),
             matcher.add.bind(matcher, '*.*.rabbit', 'Q2'),
             matcher.add.bind(matcher, 'lazy.#', 'Q2')],
            async.mapSeries.bind(async,
                ['quick.orange.rabbit',
                 'lazy.orange.elephant',
                 'quick.orange.fox',
                 'lazy.brown.fox',
                 'lazy.pink.rabbit',
                 'quick.brown.fox',
                 'orange',
                 'quick.orange.male.rabbit',
                 'lazy.orange.male.rabbit'],
                matcher.match,
                function (err, vals)
                {
                    expect(vals).to.eql(
                        [['Q1', 'Q2'],
                         ['Q1', 'Q2'],
                         ['Q1'],
                         ['Q2'],
                         ['Q2'],
                         [],
                         [],
                         [],
                         ['Q2']]);
                    done();
                }));
    });
});

