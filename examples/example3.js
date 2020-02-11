const assert = require('assert');
const { Qlobber } = require('..').set_native(require('../native'));
const matcher = new Qlobber.nativeString();

(async () => {
    await matcher.addP('foo.*', 'it matched!');
    assert.deepEqual(await matcher.matchP('foo.bar'), ['it matched!']);
    assert(await matcher.testP('foo.bar', 'it matched!'));
})();
