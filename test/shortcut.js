/*jshint node: true, mocha: true */
"use strict";

var expect = require('chai').expect,
    qlobber = require('..'),
    QlobberDedup = qlobber.QlobberDedup;

describe('shortcut', function ()
{
    it('should add shortcut when adding', function ()
    {
        var matcher = new QlobberDedup();
        expect(matcher._shortcuts.size).to.equal(0);
        matcher.add('a.b.c.d', 90);
        expect(matcher._shortcuts.size).to.equal(1);
        expect(matcher._shortcuts.get('a.b.c.d')).not.to.equal(undefined);
    });

    it('should remove shortcut when removing', function ()
    {
        var matcher = new QlobberDedup();
        expect(matcher._shortcuts.size).to.equal(0);
        matcher.add('a.b.c.d', 90);
        matcher.remove('a.b.c.d', 90);
        expect(matcher._shortcuts.size).to.equal(0);
    });

    it('should clear shortcuts when matcher is cleared', function ()
    {
        var matcher = new QlobberDedup();
        expect(matcher._shortcuts.size).to.equal(0);
        matcher.add('a.b.c.d', 90);
        matcher.clear();
        expect(matcher._shortcuts.size).to.equal(0);

    });
});
