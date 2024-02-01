(async () => {

const { workerData, parentPort } = require('worker_threads');
const { expect } = await import('chai');
const qlobber = require('../..');

qlobber.set_native(require('../../native'));

const QlobberDedup = qlobber.QlobberDedup.nativeString;

const sleep = require('util').promisify(setTimeout);
require('../rabbitmq.js');

let matcher;

if ((workerData.operation !== 'add') &&
    (workerData.operation !== 'match_one')) {
    matcher = new QlobberDedup(workerData.state_address);
    expect(matcher._ref_count).to.equal(2);
}

async function finish() {
    const uid = matcher._uid;
    matcher = null;
    while (!QlobberDedup._deletions.has(uid)) {
        gc();
        await(sleep(1000));
    }
}

switch (workerData.operation) {
case 'match':
    rabbitmq_expected_results_before_remove.forEach(function (test)
    {
        expect(Array.from(matcher.match(test[0])).sort(), test[0]).to.eql(
               test[1].sort());
        for (var v of test[1])
        {
            expect(matcher.test(test[0], v)).to.equal(true);
        }
        expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
    });
    finish();
    break;

case 'match_one':
    parentPort.once('message', state_address => {
        matcher = new QlobberDedup(state_address);
        parentPort.once('message', test => {
            parentPort.postMessage(matcher.match(test));
            finish();
        })
        parentPort.postMessage(matcher._ref_count);
    });
    parentPort.postMessage(null);
    break;

case 'remove':
    rabbitmq_bindings_to_remove.forEach(function (i)
    {
        matcher.remove(rabbitmq_test_bindings[i-1][0],
                       rabbitmq_test_bindings[i-1][1]);
    });
    parentPort.postMessage(null);
    parentPort.once('message', () => {
        expect(matcher.get_trie().size).to.equal(0);

        rabbitmq_expected_results_after_clear.forEach(function (test)
        {
            expect(Array.from(matcher.match(test[0])).sort(), test[0]).to.eql(
                   test[1].sort());
            for (var v of test[1])
            {
                expect(matcher.test(test[0], v)).to.equal(true);
            }
            expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
        });

        finish();
    });
    break;

case 'add':
    matcher = new QlobberDedup();
    expect(matcher._ref_count).to.equal(1);
    rabbitmq_test_bindings.forEach(function (topic_val)
    {
        matcher.add(topic_val[0], topic_val[1]);
    });
    parentPort.postMessage(matcher.state_address);
    matcher.add_ref(); // stop state being deleted
    finish();
    break;

default:
    throw new Error(`unknown operation: ${workerData.operation}`);
}

})();
