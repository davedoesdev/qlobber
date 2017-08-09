var QlobberSub = require('../qlobber-sub.js');
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

times(QlobberSub, add, remove, match);