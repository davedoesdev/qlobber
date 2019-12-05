const Qlobber = require('..').Qlobber.nativeString;
const {
    Worker, isMainThread, parentPort, workerData
} = require('worker_threads');

if (isMainThread) {
    const matcher = new Qlobber();
    matcher.add('foo.*', 'it matched!');
    const worker = new Worker(__filename, {
        workerData: matcher.state_address
    });
    worker.on('message', msg => {
        const assert = require('assert');
        assert.deepEqual(msg, [['it matched!'], true]);
    });
} else {
    const matcher = new Qlobber(workerData);
    parentPort.postMessage([
        matcher.match('foo.bar'),
        matcher.test('foo.bar', 'it matched!')
    ]);
}
