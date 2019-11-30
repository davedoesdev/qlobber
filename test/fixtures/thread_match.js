const { workerData } = require('worker_threads');
const expect = require('chai').expect;
const QlobberDedup = require('../..').QlobberDedup.nativeString;
require('../rabbitmq.js');

const matcher = new QlobberDedup(workerData);

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
