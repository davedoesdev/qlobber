/*jshint node: true, mocha: true */
"use strict";

var expect = require('chai').expect,
    qlobber = require('..'),
    QlobberTrue = qlobber.QlobberTrue;

describe('true', function ()
{
    it('should add and test', function ()
    {
        var matcher = new QlobberTrue();
        matcher.add('a.b.c.d');
        expect(matcher.test('a.b.c.d')).to.equal(true);
        expect(matcher.test('foo.bar')).to.equal(false);
    });

    it('should add only true when adding multiple values', function ()
    {
        var matcher = new QlobberTrue();
        matcher.add('a.b.c.d', 'foo');
        matcher.add('a.b.c.d', 'bar');
        expect(matcher.test('a.b.c.d')).to.equal(true);
        expect(matcher.test('foo.bar')).to.equal(false);
    });

    it('should be able to remove', function ()
    {
        var matcher = new QlobberTrue();
        matcher.add('a.b.c.d');
        expect(matcher.test('a.b.c.d')).to.equal(true);
        matcher.remove('a.b.c.d');
        expect(matcher.test('a.b.c.d')).to.equal(false);
    });

    it('match should call test', function ()
    {
        var matcher = new QlobberTrue();
        matcher.add('a.b.c.d');
        matcher.add('a.b.c.*');
        expect(matcher.match('a.b.c.d')).to.equal(true);
        expect(matcher.match('a.b.c.e')).to.equal(true);
        expect(matcher.match('a.b.e')).to.equal(false);
        matcher.remove('a.b.c.d');
        expect(matcher.match('a.b.c.d')).to.equal(true);
    });

    it('should work with shortcuts', function ()
    {
        var matcher = new QlobberTrue({ cache_adds: true });
        matcher.add('a.b.c.d');
        expect(matcher.test('a.b.c.d')).to.equal(true);
        matcher.remove('a.b.c.d', 'foobar');
        expect(matcher.test('a.b.c.d')).to.equal(false);
    });
});
