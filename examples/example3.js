var assert = require('assert');
var Qlobber = require('..').Qlobber.nativeString;
var matcher = new Qlobber();

(async () => {
    await matcher.addP('foo.*', 'it matched!');
    assert.deepEqual(await matcher.matchP('foo.bar'), ['it matched!']);
    assert(await matcher.testP('foo.bar', 'it matched!'));
})();
