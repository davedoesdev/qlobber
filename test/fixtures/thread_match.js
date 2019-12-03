const { workerData, parentPort } = require('worker_threads');
const expect = require('chai').expect;
const QlobberDedup = require('../..').QlobberDedup.nativeString;
const sleep = require('util').promisify(setTimeout);
require('../rabbitmq.js');

let matcher = new QlobberDedup(workerData.state_address);
expect(matcher._ref_count).to.equal(2);

async function finish() {
    const uid = matcher._uid;
    matcher = null;
    gc();
    while (!QlobberDedup._deletions.has(uid)) {
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

default:
    throw new Error(`unknown operation: ${workerData.operation}`);
}
