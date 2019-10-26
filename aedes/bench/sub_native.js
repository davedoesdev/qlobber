var QlobberSubNative = require('../qlobber-sub.js').native;
var times = require('./common');

function add(matcher, clientId, topic, qos)
{
    matcher.add(topic,
    {
        clientId: clientId,
        topic: topic,
        qos: qos
    });
}

function remove(matcher, clientId, topic, qos)
{
    matcher.remove(topic,
    {
        clientId: clientId,
        topic: topic
    });
}

function match(matcher, topic)
{
    return matcher.match(topic);
}

function test(matcher, clientId, topic)
{
    return matcher.test(topic, { clientId: clientId, topic: topic });
}

times(QlobberSubNative, add, remove, match, test);
