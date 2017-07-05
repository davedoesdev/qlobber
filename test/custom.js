/*jshint node: true, mocha: true */
"use strict";

var util = require('util'),
    expect = require('chai').expect,
    qlobber = require('..'),
    Qlobber = qlobber.Qlobber;

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

QosQlobber.prototype._add_values = function (dest, origin)
{
    origin.forEach(function (val, key)
    {
        dest.set(key, val);
    });
};

QosQlobber.prototype._remove_value = function (vals, val)
{
    vals.delete(val.clientId);
};

QosQlobber.prototype.match = function (topic)
{
    return this._match2(new Map(), topic);
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
        });

        it('should dedup multiple values with same topic and same key', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            matcher.add('foo.bar', { clientId: 'test1', qos: 20 });
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test1', { clientId: 'test1', qos: 20 } ]
            ]));
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
        });

        it('should dedup multiple values with different topics and same key', function ()
        {
            var matcher = new QosQlobber();
            matcher.add('foo.bar', { clientId: 'test1', qos: 10 });
            matcher.add('foo.*', { clientId: 'test1', qos: 20 });
            expect(matcher.match('foo.bar')).to.eql(new Map([
                [ 'test1', { clientId: 'test1', qos: 20 } ]
            ]));
        });
    });
});
