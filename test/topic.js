/*globals rabbitmq_test_bindings : false,
          rabbitmq_bindings_to_remove : false,
          rabbitmq_expected_results_before_remove: false,
          rabbitmq_expected_results_after_remove : false,
          rabbitmq_expected_results_after_remove_all : false,
          rabbitmq_expected_results_after_clear : false,
          describe: false,
          beforeEach: false,
          it: false */
/*jslint node: true */
"use strict";

var expect = require('chai').expect,
    Qlobber = require('..').Qlobber,
    common = require('./common');

function test(type, Qlobber)
{

describe(`qlobber (${type})`, function ()
{
    var matcher;

    beforeEach(function (done)
    {
        matcher = new Qlobber();
        done();
    });

    function add_bindings(bindings, mapper)
    {
        mapper = mapper || function (topic) { return topic; };

        bindings.forEach(function (topic_val)
        {
            matcher.add(topic_val[0], mapper(topic_val[1]));
        });
    }

    function remove_duplicates_filter(item, index, arr)
    {
        return item !== arr[index - 1];
    }

    Array.prototype.remove_duplicates = function ()
    {
        return this.sort().filter(remove_duplicates_filter);
    };

    function get_shortcuts(matcher)
    {
        var k, r = {}, shortcuts = matcher._shortcuts;
        for (k of shortcuts.keys())
        {
            r[k] = shortcuts.get(k);
        }
        return r;
    }

    it('should support adding bindings', function ()
    {
        add_bindings(rabbitmq_test_bindings);
        expect(common.get_trie(matcher)).to.eql(common.expected_trie);
    });

    it('should pass rabbitmq test', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        rabbitmq_expected_results_before_remove.forEach(function (test)
        {
            expect(matcher.match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });
    });

    it('should support removing bindings', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        rabbitmq_bindings_to_remove.forEach(function (i)
        {
            matcher.remove(rabbitmq_test_bindings[i-1][0],
                           rabbitmq_test_bindings[i-1][1]);
        });

        expect(common.get_trie(matcher)).to.eql({"a":{"b":{"c":{".":["t20"]},"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},"#":{".":["t12"]}}},"#":{"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]}});

        rabbitmq_expected_results_after_remove.forEach(function (test)
        {
            expect(matcher.match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });
        
        /*jslint unparam: true */
        var remaining = rabbitmq_test_bindings.filter(function (topic_val, i)
        {
            return rabbitmq_bindings_to_remove.indexOf(i + 1) < 0;
        });
        /*jslint unparam: false */

        remaining.forEach(function (topic_val)
        {
            matcher.remove(topic_val[0], topic_val[1]);
        });
            
        expect(matcher.get_trie().size).to.equal(0);

        rabbitmq_expected_results_after_clear.forEach(function (test)
        {
            expect(matcher.match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });
    });

    it('should support clearing the bindings', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        matcher.clear();

        rabbitmq_expected_results_after_clear.forEach(function (test)
        {
            expect(matcher.match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });
    });

    it('should support removing all values for a topic', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        rabbitmq_bindings_to_remove.forEach(function (i)
        {
            matcher.remove(rabbitmq_test_bindings[i-1][0]);
        });
        
        expect(common.get_trie(matcher)).to.eql({"a":{"b":{"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},"#":{".":["t12"]}}},"#":{"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]}});

        rabbitmq_expected_results_after_remove_all.forEach(function (test)
        {
            expect(matcher.match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });
    });

    if (!Qlobber.is_native)
    {
        it('should support functions as values', function ()
        {
            add_bindings(rabbitmq_test_bindings, function (topic)
            {
                return function ()
                {
                    return topic;
                };
            });

            matcher.test_values = function (vals, val)
            {
                for (var v of vals)
                {
                    if (v() === val)
                    {
                        return true;
                    }
                }

                return false;
            };

            matcher.equals_value = function (matched_value, test_value)
            {
                return matched_value() === test_value;
            };

            rabbitmq_expected_results_before_remove.forEach(function (test)
            {
                expect(matcher.match(test[0], test[0]).map(function (f)
                {
                    return f();
                }).remove_duplicates()).to.eql(test[1].sort());
                for (var v of test[1])
                {
                    expect(matcher.test(test[0], v)).to.equal(true);
                }
                expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
            });
        });

        it('should support undefined as a value', function ()
        {
            matcher.add('foo.bar');
            matcher.add('foo.*');
            expect(matcher.match('foo.bar')).to.eql([undefined, undefined]);
            expect(matcher.test('foo.bar')).to.equal(true);
        });
    }

    it('should pass example in README', function ()
    {
        matcher.add('foo.*', 'it matched!');
        expect(matcher.match('foo.bar')).to.eql(['it matched!']);
        expect(matcher.test('foo.bar', 'it matched!')).to.equal(true);
    });

    it('should pass example in rabbitmq topic tutorial', function ()
    {
        matcher.add('*.orange.*', 'Q1');
        matcher.add('*.*.rabbit', 'Q2');
        matcher.add('lazy.#', 'Q2');
        expect(['quick.orange.rabbit',
                'lazy.orange.elephant',
                'quick.orange.fox',
                'lazy.brown.fox',
                'lazy.pink.rabbit',
                'quick.brown.fox',
                'orange',
                'quick.orange.male.rabbit',
                'lazy.orange.male.rabbit'].map(function (topic)
                {
                    return [matcher.match(topic).sort(),
                            matcher.test(topic, 'Q1'),
                            matcher.test(topic, 'Q2')];
                })).to.eql(
               [[['Q1', 'Q2'], true, true],
                [['Q1', 'Q2'], true, true],
                [['Q1'], true, false],
                [['Q2'], false, true],
                [['Q2', 'Q2'], false, true],
                [[], false, false],
                [[], false, false],
                [[], false, false],
                [['Q2'], false, true]]);
    });

    it('should not remove anything if not previously added', function ()
    {
        matcher.add('foo.*', 'it matched!');
        matcher.remove('foo');
        matcher.remove('foo.*', 'something');
        matcher.remove('bar.*');
        expect(matcher.match('foo.bar')).to.eql(['it matched!']);
        expect(matcher.test('foo.bar', 'it matched!')).to.equal(true);
    });

    it('should accept wildcards in match topics', function ()
    {
        matcher.add('foo.*', 'it matched!');
        matcher.add('foo.#', 'it matched too!');
        expect(matcher.match('foo.*').sort()).to.eql(['it matched too!', 'it matched!']);
        expect(matcher.match('foo.#').sort()).to.eql(['it matched too!', 'it matched!']);
        expect(matcher.test('foo.*', 'it matched!')).to.equal(true);
        expect(matcher.test('foo.*', 'it matched too!')).to.equal(true);
        expect(matcher.test('foo.#', 'it matched!')).to.equal(true);
        expect(matcher.test('foo.#', 'it matched too!')).to.equal(true);
    });

    it('should be configurable', function ()
    {
        matcher = new Qlobber({
            separator: '/',
            wildcard_one: '+',
            wildcard_some: 'M'
        });

        matcher.add('foo/+', 'it matched!');
        matcher.add('foo/M', 'it matched too!');
        expect(matcher.match('foo/bar').sort()).to.eql(['it matched too!', 'it matched!']);
        expect(matcher.match('foo/bar/end').sort()).to.eql(['it matched too!']);
        expect(matcher.test('foo/bar', 'it matched!')).to.equal(true);
        expect(matcher.test('foo/bar', 'it matched too!')).to.equal(true);
        expect(matcher.test('foo/bar/end', 'it matched too!')).to.equal(true);
    });

    it('should match expected number of topics', function ()
    {
        if (Qlobber.is_native)
        {
            matcher = new Qlobber.nonNative.nativeNumber();
        }

        // under coverage this takes longer
        this.timeout(60000);

        var i, j, vals;

        for (i = 0; i < 60000; i += 1)
        {
            for (j = 0; j < 5; j += 1)
            {
                matcher.add('app.test.user.behrad.testTopic-' + j, i);
            }
            matcher.add('app.test.user.behrad.*', i);
        }

        vals = matcher.match('app.test.user.behrad.testTopic-0');
        expect(vals.length).to.equal(120000);
        expect(vals.remove_duplicates().length).to.equal(60000);

        expect(matcher.test('app.test.user.behrad.testTopic-0', 0)).to.equal(true);
        expect(matcher.test('app.test.user.behrad.testTopic-0', 59999)).to.equal(true);
        expect(matcher.test('app.test.user.behrad.testTopic-0', 60000)).to.equal(false);
    });

    it('should visit trie', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        let objs = [];

        for (let v of matcher.visit())
        {
            objs.push(v);
        }
        
        if (Qlobber.is_native)
        {
            expect(common.ordered_sort(objs)).to.eql(common.ordered_sort(common.expected_visits));
        }
        else
        {
            expect(objs).to.eql(common.expected_visits);
        }
    });

    it('should restore trie', function ()
    {
        let restorer = matcher.get_restorer();

        for (let v of common.expected_visits)
        {
            restorer(v);
        }

        expect(common.get_trie(matcher)).to.eql(common.expected_trie);

        rabbitmq_expected_results_before_remove.forEach(function (test)
        {
            expect(matcher.match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });
    });

    it('should not restore empty entries', function ()
    {
        const expected_visits = [
            { type: 'start_entries' },
            { type: 'entry', key: 'foo' },
            { type: 'start_entries' },
            { type: 'entry', key: 'bar' },
            { type: 'start_entries' },
            { type: 'entry', key: '.' },
            { type: 'start_values' },
            { type: 'end_values' },
            { type: 'end_entries' },
            { type: 'end_entries' },
            { type: 'end_entries' }
        ];

        if (!Qlobber.is_native)
        {
            matcher.add('foo.bar', 90);

            matcher._trie.get('foo').get('bar').get('.').shift();

            let objs = [];

            for (let v of matcher.visit())
            {
                objs.push(v);
            }

            expect(objs).to.eql(expected_visits);
        }

        let Matcher = Qlobber.is_native ? Qlobber.nonNative.nativeNumber : Qlobber,
            matcher2 = new Matcher(),
            restorer = matcher2.get_restorer();

        for (let v of expected_visits)
        {
            restorer(v);
        }

        expect(common.get_trie(matcher2)).to.eql({});
        expect(matcher2.match('foo.bar')).to.eql([]);
        expect(matcher2.test('foo.bar', 90)).to.equal(false);
    });

    it('should restore shortcuts', function ()
    {
        matcher = new Qlobber({ cache_adds: true });
        add_bindings(rabbitmq_test_bindings);

        let shortcuts = get_shortcuts(matcher);
        expect(shortcuts).to.eql({
            'a.b.c': [ 't1', 't20' ],
            'a.*.c': [ 't2' ],
            'a.#.b': [ 't3' ],
            'a.b.b.c': [ 't4' ],
            '#': [ 't5' ],
            '#.#': [ 't6' ],
            '#.b': [ 't7' ],
            '*.*': [ 't8' ],
            'a.*': [ 't9' ],
            '*.b.c': [ 't10' ],
            'a.#': [ 't11' ],
            'a.#.#': [ 't12' ],
            'b.b.c': [ 't13' ],
            'a.b.b': [ 't14' ],
            'a.b': [ 't15' ],
            'b.c': [ 't16' ],
            '': [ 't17' ],
            '*.*.*': [ 't18' ],
            'vodka.martini': [ 't19' ],
            '*.#': [ 't21' ],
            '#.*.#': [ 't22' ],
            '*.#.#': [ 't23' ],
            '#.#.#': [ 't24' ],
            '*': [ 't25' ],
            '#.b.#': [ 't26' ]
        });

        let objs = [];
        for (let v of matcher.visit())
        {
            objs.push(v);
        }
        if (Qlobber.is_native)
        {
            expect(common.ordered_sort(objs)).to.eql(common.ordered_sort(common.expected_visits));
        }
        else
        {
            expect(objs).to.eql(common.expected_visits);
        }

        let matcher2 = new Qlobber({ cache_adds: true }),
            restorer = matcher2.get_restorer();
        for (let v of common.expected_visits)
        {
            restorer(v);
        }
        expect(get_shortcuts(matcher2)).to.eql({});

        matcher2 = new Qlobber({ cache_adds: true });
        restorer = matcher2.get_restorer({ cache_adds: true });
        for (let v of common.expected_visits)
        {
            restorer(v);
        }
        expect(get_shortcuts(matcher2)).to.eql(shortcuts);
    });

    if (!Qlobber.is_native)
    {
        it('should add shortcuts to passed in Map', function ()
        {
            var topics = new Map();
            matcher = new Qlobber({ cache_adds: topics });
            add_bindings(rabbitmq_test_bindings);
            var added = Array.from(topics.keys()).sort();
            var rtopics = new Set(rabbitmq_test_bindings.map(v => v[0]));
            expect(added).to.eql(Array.from(rtopics).sort());
        });
    }

    it('should support match iterator', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        function match(topic)
        {
            let r = [];

            for (let v of matcher.match_iter(topic))
            {
                r.push(v);
            }

            return r;
        }

        rabbitmq_expected_results_before_remove.forEach(function (test)
        {
            expect(match(test[0]).remove_duplicates(), test[0]).to.eql(test[1].sort());
        });

        matcher.clear();
        matcher.add('foo.*', 'it matched!');
        matcher.add('foo.#', 'it matched too!');
        expect(match('foo.*').sort()).to.eql(['it matched too!', 'it matched!']);
        expect(match('foo.#').sort()).to.eql(['it matched too!', 'it matched!']);
    });

    it('should throw exception for topics with many words', function () {
        const topic = new Array(1000000).join('.');
        expect(() => matcher.add(topic, 'foo'), 'add').to.throw('too many words');
        expect(() => matcher.remove(topic, 'foo'), 'remove').to.throw('too many words');
        expect(() => matcher.match(topic), 'match').to.throw('too many words');
        expect(() => {
            for (let v of matcher.match_iter(topic)) {}
        }, 'match_iter').to.throw('too many words');
        expect(() => matcher.test(topic, 'foo'), 'test').to.throw('too many words');
    });

    it('should be able to change max words', function () {
        let topic = new Array(101).join('.');
        expect(() => matcher.add(topic, 'foo'), 'add').to.throw('too many words');
        expect(() => matcher.remove(topic, 'foo'), 'remove').to.throw('too many words');
        expect(() => matcher.match(topic), 'match').to.throw('too many words');
        expect(() => {
            for (let v of matcher.match_iter(topic)) {}
        }, 'match_iter').to.throw('too many words');
        expect(() => matcher.test(topic, 'foo'), 'test').to.throw('too many words');

        matcher = new Qlobber({ max_words: 101 });
        matcher.add(topic, 'foo');
        matcher.remove(topic, 'foo');
        matcher.match(topic);
        for (let v of matcher.match_iter(topic)) {}
        matcher.test(topic, 'foo');
    });

    it('should throw exception for topics with many wildcard somes', function () {
        const topic = new Array(4).fill('#').join('.');
        expect(() => matcher.add(topic, 'foo')).to.throw('too many wildcard somes');
        matcher.remove(topic, 'foo');
        matcher.match(topic);
        for (let v of matcher.match_iter(topic)) {}
        matcher.test(topic, 'foo');
    });

    it('should be able to change max wildcard somes', function () {
        matcher = new Qlobber({ max_wildcard_somes: 4 });
        const topic = new Array(4).fill('#').join('.');
        matcher.add(topic, 'foo');
    });

    it('should recurse expected number of times', function () {
        if (!Qlobber.is_native) {
            matcher = new class extends Qlobber {
                _reset_counters() {
                    this._counters = {
                        add: 0,
                        remove: 0,
                        match: 0,
                        match_some: 0,
                        match_iter: 0,
                        match_some_iter: 0,
                        test: 0,
                        test_some: 0
                    };
                }

                _add(...args) {
                    ++this._counters.add;
                    return super._add(...args);
                }

                _remove(...args) {
                    ++this._counters.remove;
                    return super._remove(...args);
                }

                _match(...args) {
                    ++this._counters.match;
                    return super._match(...args);
                }

                _match_some(...args) {
                    ++this._counters.match_some;
                    return super._match_some(...args);
                }

                _match_iter(...args) {
                    ++this._counters.match_iter;
                    return super._match_iter(...args);
                }

                _match_some_iter(...args) {
                    ++this._counters.match_some_iter;
                    return super._match_some_iter(...args);
                }

                _test(...args) {
                    ++this._counters.test;
                    return super._test(...args);
                }

                _test_some(...args) {
                    ++this._counters.test_some;
                    return super._test_some(...args);
                }
            }();
        }

        function check(pattern, topic,
                       eadd,
                       ematch, 
                       ematch_some,
                       etest,
                       etest_some,
                       eremove) {
            if (Array.isArray(pattern)) {
                pattern = pattern.join('.');
            }
            if (Array.isArray(topic)) {
                topic = topic.join('.');
            }

            matcher.clear();
            matcher._reset_counters();

            matcher.add(pattern, 'foo');
            expect(matcher._counters.add).to.equal(eadd);

            matcher.match(topic);
            expect(matcher._counters.match).to.equal(ematch);
            expect(matcher._counters.match_some).to.equal(ematch_some);

            for (let v of matcher.match_iter(topic)) {}
            expect(matcher._counters.match_iter).to.equal(ematch);
            expect(matcher._counters.match_some_iter).to.equal(ematch_some);

            matcher.test(topic, 'foo');
            expect(matcher._counters.test).to.equal(etest);
            expect(matcher._counters.test_some).to.equal(etest_some);

            matcher.remove(pattern, 'foo');
            expect(matcher._counters.remove).to.equal(eremove);
        }

        check(new Array(100),
              new Array(100),
              101, 101, 0, 101, 0, 101);

        check(new Array(100).fill('*'),
              new Array(100),
              101, 1, 0, 1, 0, 101);

        check(new Array(100).fill('*'),
              new Array(100).fill('xyz'),
              101, 101, 0, 101, 0, 101);

        check(new Array(1).fill('#'),
              new Array(100).fill('xyz'),
              2, 2, 1, 2, 1, 2);
              
        check(new Array(3).fill('#'),
              new Array(100).fill('xyz'),
              4, 10404, 5253, 4, 3, 4);

        check(new Array(3).fill('#').concat('xyz'),
              new Array(100).fill('xyz'),
              5, 353804, 5253, 203, 3, 5);

        check('xyz.#.xyz.#.xyz.#.xyz',
              new Array(100).fill('xyz'),
              8, 328551, 4951, 200, 3, 8);
    });
});

}

test('non-native', Qlobber);
test('native string', Qlobber.nativeString);
