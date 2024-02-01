/*jshint node: true, mocha: true */
"use strict";

let expect;
const { QlobberDedup } = require('..');

before(async () => {
    ({ expect } = await import('chai'));
});

describe('topic split cache', function () {
    it('should add to cache when adding', function () {
        const matcher = new QlobberDedup({ cache_splits: 1000 });
        expect(matcher._split_cache.size).to.equal(0);
        matcher.add('a.b.c.d', 90);
        expect(matcher._split_cache.size).to.equal(1);
        expect(matcher._split_cache.get('a.b.c.d')).to.eql([
            'a', 'b', 'c', 'd'
        ]);
        expect(Array.from(matcher.match('a.b.c.d'))).to.eql([90]);
        expect(matcher.test('a.b.c.d', 90)).to.equal(true);
        expect(matcher._split_cache.size).to.equal(1);
        expect(matcher._split_cache.get('a.b.c.d')).to.eql([
            'a', 'b', 'c', 'd'
        ]);
    });

    it('should add to cache when matching', function () {
        const matcher = new QlobberDedup({ cache_splits: 1000 });
        expect(matcher._split_cache.size).to.equal(0);
        expect(Array.from(matcher.match('a.b.c.d'))).to.eql([]);
        expect(matcher._split_cache.size).to.equal(1);
        expect(matcher._split_cache.get('a.b.c.d')).to.eql([
            'a', 'b', 'c', 'd'
        ]);
    });

    it('should evict oldest entry from cache', function () {
        const matcher = new QlobberDedup({ cache_splits: 3 });
        expect(matcher._split_cache.size).to.equal(0);
        matcher.add('a.b.c.d', 90);
        matcher.match('a.b.c.d');
        matcher.add('a.b.c.e', 90);
        matcher.match('a.b.c.e');
        matcher.add('a.b.c.f', 90);
        matcher.match('a.b.c.f');
        expect(matcher._split_cache.size).to.equal(3);
        expect(matcher._split_cache.get('a.b.c.d')).to.eql([
            'a', 'b', 'c', 'd'
        ]);
        expect(matcher._split_cache.get('a.b.c.e')).to.eql([
            'a', 'b', 'c', 'e'
        ]);
        expect(matcher._split_cache.get('a.b.c.f')).to.eql([
            'a', 'b', 'c', 'f'
        ]);
        matcher.add('a.b.c.g', 90);
        expect(matcher._split_cache.size).to.equal(3);
        expect(matcher._split_cache.get('a.b.c.d')).to.equal(undefined);
        expect(matcher._split_cache.get('a.b.c.e')).to.eql([
            'a', 'b', 'c', 'e'
        ]);
        expect(matcher._split_cache.get('a.b.c.f')).to.eql([
            'a', 'b', 'c', 'f'
        ]);
        expect(matcher._split_cache.get('a.b.c.g')).to.eql([
            'a', 'b', 'c', 'g'
        ]);
    });

    it('should clear cache when matcher is cleared', function () {
        const matcher = new QlobberDedup({ cache_splits: 3 });
        expect(matcher._split_cache.size).to.equal(0);
        matcher.add('a.b.c.d', 90);
        expect(matcher._split_cache.size).to.equal(1);
        matcher.clear();
        expect(matcher._split_cache.size).to.equal(0);
    });
});
