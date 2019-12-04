/*globals rabbitmq_test_bindings : false,
          rabbitmq_bindings_to_remove : false,
          rabbitmq_expected_results_before_remove: false,
          rabbitmq_expected_results_after_remove : false,
          rabbitmq_expected_results_after_remove_all : false,
          rabbitmq_expected_results_after_clear : false,
          describe: false,
          beforeEach: false,
          it: false */
/*jslint node: true */
"use strict";

var path = require('path'),
    expect = require('chai').expect,
    QlobberDedup = require('..').QlobberDedup.nativeString,
    common = require('./common'),
    { Worker } = require('worker_threads'),
    { promisify } = require('util');

async function wait_for_worker(worker) {
    if (!worker.exited) {
        await promisify(cb => {
            worker.on('error', cb);

            worker.on('exit', code => {
                cb(code === 0 ? null : new Error(`worker exited with code ${code}`));
            });
        })();
    }
}

describe('qlobber-threads', function ()
{
    var matcher;

    beforeEach(function (done)
    {
        matcher = new QlobberDedup();
        done();
    });

    function add_bindings(bindings)
    {
        bindings.forEach(function (topic_val)
        {
            matcher.add(topic_val[0], topic_val[1]);
        });
    }

    it('should share state between matchers', function ()
    {
        add_bindings(rabbitmq_test_bindings);
        expect(common.get_trie(matcher)).to.eql(common.expected_trie);
        expect(matcher._ref_count).to.equal(1);

        const matcher2 = new QlobberDedup(matcher.state_address);
        expect(Buffer.from(matcher2.state_address).equals(
               Buffer.from(matcher.state_address))).to.equal(true);
        expect(common.get_trie(matcher2)).to.eql(common.expected_trie);
        expect(matcher._ref_count).to.equal(2);
        expect(matcher2._ref_count).to.equal(2);
    });

    it('should add on one matcher and match on another', function ()
    {
        add_bindings(rabbitmq_test_bindings);

        const matcher2 = new QlobberDedup(matcher.state_address);

        rabbitmq_expected_results_before_remove.forEach(function (test)
        {
            expect(Array.from(matcher2.match(test[0])).sort(), test[0]).to.eql(
                   test[1].sort());
            for (var v of test[1])
            {
                expect(matcher2.test(test[0], v)).to.equal(true);
            }
            expect(matcher2.test(test[0], 'xyzfoo')).to.equal(false);
        });
    });
    
    it('should add on one thread and match on another', async function ()
    {
        this.timeout(60000);

        expect(matcher._ref_count).to.equal(1);
        add_bindings(rabbitmq_test_bindings);

        await wait_for_worker(new Worker(
            path.join(__dirname, 'fixtures', 'worker.js'), {
                workerData: {
                    state_address: matcher.state_address,
                    operation: 'match'
                }
            }));

        expect(matcher._ref_count).to.equal(1);
    });

    it('should share state between two threads', function (cb)
    {
        this.timeout(60000);

        expect(matcher._ref_count).to.equal(1);
        add_bindings(rabbitmq_test_bindings);

        const worker = new Worker(
            path.join(__dirname, 'fixtures', 'worker.js'), {
                workerData: {
                    state_address: matcher.state_address,
                    operation: 'remove'
                }
            });
        worker.on('exit', () => worker.exited = true);

        worker.once('message', async () => {
            expect(common.get_trie(matcher)).to.eql({"a":{"b":{"c":{".":["t20"]},"b":{"c":{".":["t4"]},".":["t14"]},".":["t15"]},"*":{"c":{".":["t2"]},".":["t9"]},"#":{"b":{".":["t3"]},"#":{".":["t12"]}}},"#":{"#":{".":["t6"],"#":{".":["t24"]}},"b":{".":["t7"],"#":{".":["t26"]}},"*":{"#":{".":["t22"]}}},"*":{"*":{".":["t8"],"*":{".":["t18"]}},"b":{"c":{".":["t10"]}},"#":{"#":{".":["t23"]}},".":["t25"]},"b":{"b":{"c":{".":["t13"]}},"c":{".":["t16"]}},"":{".":["t17"]}});

            rabbitmq_expected_results_after_remove.forEach(function (test)
            {
                expect(Array.from(matcher.match(test[0])).sort(), test[0]).to.eql(
                       test[1].sort());
                for (var v of test[1])
                {
                    expect(matcher.test(test[0], v)).to.equal(true);
                }
                expect(matcher.test(test[0], 'xyzfoo')).to.equal(false);
            });
            
            /*jslint unparam: true */
            var remaining = rabbitmq_test_bindings.filter(function (topic_val, i)
            {
                return rabbitmq_bindings_to_remove.indexOf(i + 1) < 0;
            });
            /*jslint unparam: false */

            remaining.forEach(function (topic_val)
            {
                matcher.remove(topic_val[0], topic_val[1]);
            });

            worker.postMessage(null);
            await wait_for_worker(worker);
            expect(matcher._ref_count).to.equal(1);
            cb();
        });
    });

    it('should share state between many threads', async function () {
        this.timeout(2 * 60 * 1000);

        const workers = await Promise.all(
            rabbitmq_expected_results_before_remove.map(() => {
                const worker = new Worker(
                    path.join(__dirname, 'fixtures', 'worker.js'), {
                        workerData: {
                            operation: 'match_one'
                        }
                    });
                worker.on('exit', () => worker.exited = true);

                return promisify(cb => {
                    worker.once('message', () => cb(null, worker));
                })();
            }));

        const add_worker = new Worker(
            path.join(__dirname, 'fixtures', 'worker.js'), {
                workerData: {
                    operation: 'add'
                }
            });
        add_worker.on('exit', () => add_worker.exited = true);
        const state_address = await promisify(cb => {
            add_worker.once('message', v => cb(null, v));
        })();
        await wait_for_worker(add_worker);

        const ref_counts = await Promise.all(
            rabbitmq_expected_results_before_remove.map((test, i) => {
                workers[i].postMessage(state_address);
                return promisify(cb => {
                    workers[i].once('message', n => cb(null, n));
                })();
            }));
        // extra ref was added before state address was sent
        expect(Math.max(...ref_counts)).to.equal(workers.length + 1);

        await Promise.all(
            rabbitmq_expected_results_before_remove.map((test, i) => {
                workers[i].postMessage(test[0]);
                return promisify(cb => {
                    workers[i].once('message', r => {
                        expect(Array.from(r).sort(), test[0]).to.eql(
                               test[1].sort());
                        cb();
                    });
                })();
            }));

        for (const worker of workers) {
            await wait_for_worker(worker);
        }

        const matcher = new QlobberDedup(state_address);
        expect(matcher._ref_count).to.equal(2);
        matcher.release();
        expect(matcher._ref_count).to.equal(1);
    });
});
