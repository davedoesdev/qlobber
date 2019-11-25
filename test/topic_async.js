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
            r.push([(await matcher.matchP(topic)).sort(),
                    await matcher.testP(topic, 'Q1'),
                    await matcher.testP(topic, 'Q2')]);
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

    it('should not remove anything if not previously added', async function ()
    {
        await matcher.addP('foo.*', 'it matched!');
        await matcher.removeP('foo');
        await matcher.removeP('foo.*', 'something');
        await matcher.removeP('bar.*');
        expect(await matcher.matchP('foo.bar')).to.eql(['it matched!']);
        expect(await matcher.testP('foo.bar', 'it matched!')).to.equal(true);
    });

    it('should accept wildcards in match topics', async function ()
    {
        await matcher.addP('foo.*', 'it matched!');
        await matcher.addP('foo.#', 'it matched too!');
        expect((await matcher.matchP('foo.*')).sort()).to.eql(['it matched too!', 'it matched!']);
        expect((await matcher.matchP('foo.#')).sort()).to.eql(['it matched too!', 'it matched!']);
        expect(await matcher.testP('foo.*', 'it matched!')).to.equal(true);
        expect(await matcher.testP('foo.*', 'it matched too!')).to.equal(true);
        expect(await matcher.testP('foo.#', 'it matched!')).to.equal(true);
        expect(await matcher.testP('foo.#', 'it matched too!')).to.equal(true);
    });

    it('should be configurable', async function ()
    {
        matcher = new Qlobber({
            separator: '/',
            wildcard_one: '+',
            wildcard_some: 'M'
        });

        await matcher.addP('foo/+', 'it matched!');
        await matcher.addP('foo/M', 'it matched too!');
        expect((await matcher.matchP('foo/bar')).sort()).to.eql(['it matched too!', 'it matched!']);
        expect((await matcher.matchP('foo/bar/end')).sort()).to.eql(['it matched too!']);
        expect(await matcher.testP('foo/bar', 'it matched!')).to.equal(true);
        expect(await matcher.testP('foo/bar', 'it matched too!')).to.equal(true);
        expect(await matcher.testP('foo/bar/end', 'it matched too!')).to.equal(true);
    });

    it('should match expected number of topics', async function ()
    {
        matcher = new Qlobber.nonNative.nativeNumber();

        // under coverage this takes longer
        this.timeout(60000);

        var i, j, vals;

        for (i = 0; i < 60000; i += 1)
        {
            for (j = 0; j < 5; j += 1)
            {
                await matcher.addP('app.test.user.behrad.testTopic-' + j, i);
            }
            await matcher.addP('app.test.user.behrad.*', i);
        }

        vals = await matcher.matchP('app.test.user.behrad.testTopic-0');
        expect(vals.length).to.equal(120000);
        expect(vals.remove_duplicates().length).to.equal(60000);

        expect(await matcher.testP('app.test.user.behrad.testTopic-0', 0)).to.equal(true);
        expect(await matcher.testP('app.test.user.behrad.testTopic-0', 59999)).to.equal(true);
        expect(await matcher.testP('app.test.user.behrad.testTopic-0', 60000)).to.equal(false);
    });

    it('should visit trie', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);

        let objs = [];

        for await (let v of matcher.visitP())
        {
            objs.push(v);
        }
        
        expect(common.ordered_sort(objs)).to.eql(common.ordered_sort(common.expected_visits));
    });

    it('should restore trie', async function ()
    {
        let restorer = await matcher.get_restorerP();

        for (let v of common.expected_visits)
        {
            await restorer(v);
        }

        expect(common.get_trie(matcher)).to.eql(common.expected_trie);

        for (const test of rabbitmq_expected_results_before_remove)
        {
            expect((await matcher.matchP(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
            for (var v of test[1])
            {
                expect(await matcher.testP(test[0], v)).to.equal(true);
            }
            expect(await matcher.testP(test[0], 'xyzfoo')).to.equal(false);
        }
    });

    it('should restore shortcuts', async function ()
    {
        matcher = new Qlobber({ cache_adds: true });
        await add_bindings(rabbitmq_test_bindings);

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
        for await (let v of matcher.visitP())
        {
            objs.push(v);
        }
        expect(common.ordered_sort(objs)).to.eql(common.ordered_sort(common.expected_visits));

        let matcher2 = new Qlobber({ cache_adds: true }),
            restorer = await matcher2.get_restorerP();
        for (let v of common.expected_visits)
        {
            await restorer(v);
        }
        expect(get_shortcuts(matcher2)).to.eql({});

        matcher2 = new Qlobber({ cache_adds: true });
        restorer = await matcher2.get_restorerP({ cache_adds: true });
        for (let v of common.expected_visits)
        {
            await restorer(v);
        }
        expect(get_shortcuts(matcher2)).to.eql(shortcuts);
    });

    it('should support match iterator', async function ()
    {
        await add_bindings(rabbitmq_test_bindings);

        async function match(topic)
        {
            let r = [];

            for await (let v of matcher.match_iterP(topic))
            {
                r.push(v);
            }

            return r;
        }

        for (const test of rabbitmq_expected_results_before_remove)
        {
            expect((await match(test[0])).remove_duplicates(), test[0]).to.eql(test[1].sort());
        }

        await matcher.clearP();
        await matcher.addP('foo.*', 'it matched!');
        await matcher.addP('foo.#', 'it matched too!');
        expect((await match('foo.*')).sort()).to.eql(['it matched too!', 'it matched!']);
        expect((await match('foo.#')).sort()).to.eql(['it matched too!', 'it matched!']);
    });

    async function expect_throw(f, msg) {
        let ex;
        try {
            await f();
        } catch (e) {
            ex = e;
        }
        expect(ex.message).to.equal(msg);
    }

    it('should throw exception for topics with many words', async function () {
        const topic = new Array(1000000).join('.');
        await expect_throw(async () => await matcher.addP(topic, 'foo'), 'too many words');
        await expect_throw(async () => await matcher.removeP(topic, 'foo'), 'too many words');
        await expect_throw(async () => await matcher.matchP(topic), 'too many words');
        await expect_throw(async () => {
            for await (let v of matcher.match_iterP(topic)) {}
        }, 'too many words');
        await expect_throw(async () => await matcher.testP(topic, 'foo'), 'too many words');
    });

    it('should be able to change max words', async function () {
        const topic = new Array(101).join('.');
        await expect_throw(async () => await matcher.addP(topic, 'foo'), 'too many words');
        await expect_throw(async () => await matcher.removeP(topic, 'foo'), 'too many words');
        await expect_throw(async () => await matcher.matchP(topic), 'too many words');
        await expect_throw(async () => {
            for await (let v of matcher.match_iterP(topic)) {}
        }, 'too many words');
        await expect_throw(async () => await matcher.testP(topic, 'foo'), 'too many words');

        matcher = new Qlobber({ max_words: 101 });
        await matcher.addP(topic, 'foo');
        await matcher.removeP(topic, 'foo');
        await matcher.matchP(topic);
        for await (let v of matcher.match_iterP(topic)) {}
        await matcher.testP(topic, 'foo');
    });
});