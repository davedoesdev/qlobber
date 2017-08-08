var QlobberDedup = require('../..').QlobberDedup;
var times = require('./common');

function add(matcher, clientId, topic, qos)
{
    matcher.add(topic, clientId + ';' + topic + ';' + qos);
}

function remove(matcher, clientId, topic, qos)
{
    matcher.remove(topic, clientId + ';' + topic + ';' + qos);
}

function match(matcher, topic)
{
    return Array.from(matcher.match(topic)).map(function (m)
    {
        var parts = m.split(';');
        return {
            clientId: parts[0],
            topic: parts[1],
            qos: +parts[2]
        };
    });
}

times(QlobberDedup, add, remove, match);
