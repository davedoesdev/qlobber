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
    Qlobber = require('..').Qlobber.nativeString,
    common = require('./common');

describe('qlobber-async', function ()
{
    var matcher;

    beforeEach(function (done)
    {
        matcher = new Qlobber();
        done();
    });

    async function add_bindings(bindings, mapper)
    {
        mapper = mapper || function (topic) { return topic; };

        for (const topic_val of bindings)
        {
            await matcher.addP(topic_val[0], mapper(topic_val[1]));
        }
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

    it('should support adding bindings', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);
        expect(common.get_trie(matcher)).to.eql(common.expected_trie);
    });

    it('should pass rabbitmq test', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);

        for (const test of rabbitmq_expected_results_before_remove)
        {
            expect((await matcher.matchP(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (const v of test[1])
            {
                expect(await matcher.testP(test[0], v)).to.equal(true);
            }
            expect(await matcher.testP(test[0], 'xyzfoo')).to.equal(false);
        }
    });

    it('should support removing bindings', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);

        for (const i of rabbitmq_bindings_to_remove)
        {
            await matcher.removeP(rabbitmq_test_bindings[i-1][0],
                                 rabbitmq_test_bindings[i-1][1]);
        }

        expect(common.get_trie(matcher)).to.eql({"a":{"b":{"c":{".":["t20"]},"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},"#":{".":["t12"]}}},"#":{"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]}});

        for (const test of rabbitmq_expected_results_after_remove)
        {
            expect((await matcher.matchP(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (const v of test[1])
            {
                expect(await matcher.testP(test[0], v)).to.equal(true);
            }
            expect(await matcher.testP(test[0], 'xyzfoo')).to.equal(false);
        }
        
        /*jslint unparam: true */
        var remaining = rabbitmq_test_bindings.filter(function (topic_val, i)
        {
            return rabbitmq_bindings_to_remove.indexOf(i + 1) < 0;
        });
        /*jslint unparam: false */

        for (const topic_val of remaining)
        {
            await matcher.removeP(topic_val[0], topic_val[1]);
        }
            
        expect(matcher.get_trie().size).to.equal(0);

        for (const test of rabbitmq_expected_results_after_clear)
        {
            expect((await matcher.matchP(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (const v of test[1])
            {
                expect(await matcher.testP(test[0], v)).to.equal(true);
            }
            expect(await matcher.testP(test[0], 'xyzfoo')).to.equal(false);
        }
    });

    it('should support clearing the bindings', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);

        await matcher.clearP();

        for (const test of rabbitmq_expected_results_after_clear)
        {
            expect((await matcher.matchP(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (const v of test[1])
            {
                expect(await matcher.testP(test[0], v)).to.equal(true);
            }
            expect(await matcher.testP(test[0], 'xyzfoo')).to.equal(false);
        }
    });

    it('should support removing all values for a topic', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);

        for (const i of rabbitmq_bindings_to_remove)
        {
            await matcher.removeP(rabbitmq_test_bindings[i-1][0]);
        }
        
        expect(common.get_trie(matcher)).to.eql({"a":{"b":{"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},"#":{".":["t12"]}}},"#":{"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]}});

        for (const test of rabbitmq_expected_results_after_remove_all)
        {
            expect((await matcher.matchP(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(await matcher.testP(test[0], v)).to.equal(true);
            }
            expect(await matcher.testP(test[0], 'xyzfoo')).to.equal(false);
        }
    });

    it('should pass example in README', async function ()
    {
        await matcher.addP('foo.*', 'it matched!');
        expect(await matcher.matchP('foo.bar')).to.eql(['it matched!']);
        expect(await matcher.testP('foo.bar', 'it matched!')).to.equal(true);
    });

    it('should pass example in rabbitmq topic tutorial', async function ()
    {
        await matcher.addP('*.orange.*', 'Q1');
        await matcher.addP('*.*.rabbit', 'Q2');
        await matcher.addP('lazy.#', 'Q2');

        const r = [];

        for (const topic of [
            'quick.orange.rabbit',
            'lazy.orange.elephant',
            'quick.orange.fox',
            'lazy.brown.fox',
            'lazy.pink.rabbit',
            'quick.brown.fox',
            'orange',
            'quick.orange.male.rabbit',
            'lazy.orange.male.rabbit']) {
            r.push([(await matcher.match(topic)).sort(),
                    await matcher.test(topic, 'Q1'),
                    await matcher.test(topic, 'Q2')]);
        }

        expect(r).to.eql([
            [['Q1', 'Q2'], true, true],
            [['Q1', 'Q2'], true, true],
            [['Q1'], true, false],
            [['Q2'], false, true],
            [['Q2', 'Q2'], false, true],
            [[], false, false],
            [[], false, false],
            [[], false, false],
            [['Q2'], false, true]]);
    });

    return;

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
});
