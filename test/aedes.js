/*jshint node: true, mocha: true */
"use strict";

var expect = require('chai').expect,
    QlobberSub = require('../aedes/qlobber-sub');

describe('qlobber-sub', function ()
{
    it('should add and match a single value', function ()
    {
        var matcher = new QlobberSub();
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
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
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 2
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 2
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
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        },
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
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
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        matcher.add('foo.*',
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 2
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        },
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 2
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
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
        });
        matcher.remove('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar'
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
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
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        matcher.add('foo.*',
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 1
        });
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        },
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
        },
        {
            clientId: 'test1',
            topic: 'foo.*',
            qos: 1
        }]);
        expect(matcher.match('foo.bar', 'foo.bar')).to.eql([
        {
            clientId: 'test1',
            qos: 1
        },
        {
            clientId: 'test2',
            qos: 2
        }]);
        expect(matcher.match('foo.bar', 'foo.*')).to.eql([
        {
            clientId: 'test1',
            qos: 1
        }]);
    });

    it("removing value shouldn't care about topic", function ()
    {
        var matcher = new QlobberSub();
        matcher.add('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar',
            qos: 1
        });
        matcher.add('foo.bar',
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
        });
        matcher.remove('foo.bar',
        {
            clientId: 'test1',
            topic: 'foo.bar2'
        });
        matcher.remove('foo.bar',
        {
            clientId: 'test3',
            topic: 'foo.bar'
        });
        expect(matcher.match('foo.bar')).to.eql([
        {
            clientId: 'test2',
            topic: 'foo.bar',
            qos: 2
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
});
