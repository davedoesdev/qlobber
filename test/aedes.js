/*jshint node: true, mocha: true */
"use strict";

var expect,
    QlobberSub = require('../aedes/qlobber-sub').set_native(require('../native')),
    common = require('./common');

before(async () => {
    ({ expect } = await import('chai'));
});

function test(type, QlobberSub)
{

(QlobberSub ? describe : describe.skip)(`qlobber-sub (${type})`, function ()
{
    it('should add and match a single value', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(true);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(false);
    });

    it('should dedup multiple values with same client ID and topic', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(true);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(false);
    });

    it('should not dedup multiple values with different client IDs and same topic', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        expect(common.ordered_sort(matcher.match('foo.bar'))).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        },
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(false);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(false);
    });

    it('should not dedup multiple values with same client ID and different topics', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.*',
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        },
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(true);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.*'
        })).to.equal(true);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'bar.*'
        })).to.equal(false);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(false);
    });

    it('should remove value', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        matcher.remove('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(false);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(true);
    });

    it('should be able to pass specific topic to match', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.*',
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(3);
        expect(common.ordered_sort(matcher.match('foo.bar'))).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        },
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        },
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(common.ordered_sort(matcher.match('foo.bar', 'foo.bar'))).to.eql([
        {
            clientId: 'test1',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        },
        {
            clientId: 'test2',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.match('foo.bar', 'foo.*')).to.eql([
        {
            clientId: 'test1',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        }]);
    });

    it("removing value shouldn't care about topic in value", function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        matcher.remove('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar2'
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.remove('foo.bar',
        {
            clientId: 'test3',
            topic: 'foo.bar'
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(false);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(true);
        matcher.remove('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar2'
        });
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.remove('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar2'
        });
        expect(matcher.subscriptionsCount).to.equal(0);
    });

    it('should clear matcher', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        }]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(true);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(false);
        matcher.clear();
        expect(matcher.subscriptionsCount).to.equal(0);
        expect(matcher.match('foo.bar')).to.eql([]);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        })).to.equal(false);
        expect(matcher.test('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar'
        })).to.equal(false);
    });

    it('should count client subscription if has an existing subscription and is then added to a topic which already has a subscription for another client', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.*',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        matcher.add('foo.*',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(3);
        matcher.remove('foo.*',
        {
            clientId: 'test1',
            topic: 'foo.bar',
        });
        expect(matcher.subscriptionsCount).to.equal(2);
    });

    let expected_visits = [
        { type: 'start_entries' },
        { type: 'entry', key: 'foo' },
        { type: 'start_entries' },
        { type: 'entry', key: 'bar' },
        { type: 'start_entries' },
        { type: 'entry', key: '.' },
        { type: 'start_values' },
        {
          type: 'value',
          value: { topic: 'foo.bar', clientId: 'test1', qos: 1, rh: 0, rap: true, nl: false }
        },
        {
          type: 'value',
          value: { topic: 'foo.bar', clientId: 'test2', qos: 2, rh: 0, rap: true, nl: false }
        },
        { type: 'end_values' },
        { type: 'end_entries' },
        { type: 'entry', key: 'bar2' },
        { type: 'start_entries' },
        { type: 'entry', key: '.' },
        { type: 'start_values' },
        {
          type: 'value',
          value: { topic: 'foo.bar2', clientId: 'test1', qos: 1, rh: 0, rap: true, nl: false }
        },
        {
          type: 'value',
          value: { topic: 'foo.bar2', clientId: 'test2', qos: 2, rh: 0, rap: true, nl: false }
        },
        { type: 'end_values' },
        { type: 'end_entries' },
        { type: 'end_entries' },
        { type: 'end_entries' }
    ];

    it('should visit trie', function ()
    {
        var matcher = new QlobberSub();
        expect(matcher.subscriptionsCount).to.equal(0);
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(1);
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(2);
        matcher.add('foo.bar2',
        {
            clientId: 'test1',
            topic: 'foo.bar2',
            qos: 1,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(3);
        matcher.add('foo.bar2',
        {
            clientId: 'test2',
            topic: 'foo.bar2',
            qos: 2,
            rh: 0,
            rap: true,
            nl: false
        });
        expect(matcher.subscriptionsCount).to.equal(4);

        let objs = [];

        for (let v of matcher.visit())
        {
            objs.push(v);
        }

        if (QlobberSub.is_native)
        {
            expect(common.ordered_sort(objs)).to.eql(common.ordered_sort(expected_visits));
        }
        else
        {
            expect(objs).to.eql(expected_visits);
        }
    });

    it('should restore trie', function ()
    {
        var matcher = new QlobberSub();
        let restorer = matcher.get_restorer();

        for (let v of expected_visits)
        {
            restorer(v);
        }

        expect(common.get_trie(matcher)).to.eql({"foo":{"bar2":{".":[{"topic":"foo.bar2","clientId":"test1","qos":1, rh: 0, rap: true, nl: false},{"topic":"foo.bar2","clientId":"test2","qos":2, rh: 0, rap: true, nl: false}]},"bar":{".":[{"topic":"foo.bar","clientId":"test1","qos":1, rh: 0, rap: true, nl: false},{"topic":"foo.bar","clientId":"test2","qos":2, rh: 0, rap: true, nl: false}]}}});
    });

    it('should support match iterator', function ()
    {
        var matcher = new QlobberSub();
        matcher.add('foo.bar', { clientId: 'test1', topic: 'foo.bar', qos: 1, rh: 0, rap: true, nl: false });
        matcher.add('foo.*', { clientId: 'test1', topic: 'foo.*', qos: 2, rh: 0, rap: true, nl: false });

        let expected_matches = [
            { topic: 'foo.bar', clientId: 'test1', qos: 1, rh: 0, rap: true, nl: false },
            { topic: 'foo.*', clientId: 'test1', qos: 2, rh: 0, rap: true, nl: false }
        ];

        let objs = [];
        for (let v of matcher.match_iter('foo.bar'))
        {
            objs.push(v);
        }
        if (QlobberSub.is_native)
        {
            expect(common.ordered_sort(objs)).to.eql(common.ordered_sort(expected_matches));
        }
        else
        {
            expect(objs).to.eql(expected_matches);
        }

        objs = [];
        for (let v of matcher.match_iter('foo.bar', 'foo.bar'))
        {
            objs.push(v);
        }
        expect(objs).to.eql([ { clientId: 'test1', qos: 1, rh: 0, rap: true, nl: false } ]);
    });
});

}

test('non-native', QlobberSub);
test('native', QlobberSub.native);
