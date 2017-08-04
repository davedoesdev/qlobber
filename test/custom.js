/*jshint node: true, mocha: true */
"use strict";

var util = require('util'),
    expect = require('chai').expect,
    qlobber = require('..'),
    Qlobber = qlobber.Qlobber,
    ctx = 'some context';

function QosQlobber(options)
{
    Qlobber.call(this, options);
}

util.inherits(QosQlobber, Qlobber);

QosQlobber.prototype._initial_value = function (val)
{
    return new Map().set(val.clientId, val);
};

QosQlobber.prototype._add_value = function (vals, val)
{
    vals.set(val.clientId, val);
};

QosQlobber.prototype._add_values = function (dest, origin, context)
{
    expect(context).to.equal(ctx);

    origin.forEach(function (val, key)
    {
        dest.set(key, val);
    });
};

QosQlobber.prototype._remove_value = function (vals, val)
{
    if (val === undefined)
    {
        return true;
    }

    vals.delete(val);
    return vals.size === 0;
};

QosQlobber.prototype.test_values = function (vals, val)
{
    return vals.has(val);
};

QosQlobber.prototype.match = function (topic)
{
    return this._match2(new Map(), topic, ctx);
};

describe('qlobber-custom', function ()
{
    describe('dedup objects by key', function ()
    {
        it('should add and match a single value', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test1', { clientId: 'test1', qos: 10 } ]
            ]));
            expect(matcher.test('foo.bar', 'test1')).to.equal(true);
            expect(matcher.test('foo.bar', 'test2')).to.equal(false);
        });

        it('should dedup multiple values with same topic and same key', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            matcher.add('foo.bar', { clientId: 'test1', qos: 20 });
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test1', { clientId: 'test1', qos: 20 } ]
            ]));
            expect(matcher.test('foo.bar', 'test1')).to.equal(true);
            expect(matcher.test('foo.bar', 'test2')).to.equal(false);
        });

        it('should not dedup multiple values with same topic and different keys', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            matcher.add('foo.bar', { clientId: 'test2', qos: 20 });
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test1', { clientId: 'test1', qos: 10 } ],
                [ 'test2', { clientId: 'test2', qos: 20 } ]
            ]));
            expect(matcher.test('foo.bar', 'test1')).to.equal(true);
            expect(matcher.test('foo.bar', 'test2')).to.equal(true);
        });

        it('should dedup multiple values with different topics and same key', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            matcher.add('foo.*', { clientId: 'test1', qos: 20 });
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test1', { clientId: 'test1', qos: 20 } ]
            ]));
            expect(matcher.test('foo.bar', 'test1')).to.equal(true);
            expect(matcher.test('foo.bar', 'test2')).to.equal(false);
        });

        it('should remove value', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            matcher.add('foo.bar', { clientId: 'test2', qos: 20 });
            matcher.remove('foo.bar', 'test1');
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test2', { clientId: 'test2', qos: 20 } ]
            ]));
            expect(matcher.test('foo.bar', 'test1')).to.equal(false);
            expect(matcher.test('foo.bar', 'test2')).to.equal(true);
        });
    });
});
