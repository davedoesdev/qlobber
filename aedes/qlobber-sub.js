/*jslint node: true */
"use strict";

var util = require('util'),
    Qlobber = require('..').Qlobber;

function QlobberSub (options)
{
    Qlobber.call(this, options);
}

util.inherits(QlobberSub, Qlobber);

QlobberSub.prototype._initial_value = function (val)
{
    var topicMap = new Map().set(val.topic, val.qos);
    return new Map().set(val.clientId, topicMap);
};

QlobberSub.prototype._add_value = function (clientMap, val)
{
    var topicMap = clientMap.get(val.clientId);
    if (!topicMap)
    {
        topicMap = new Map();
        clientMap.set(val.clientId, topicMap);
    }
    topicMap.set(val.topic, val.qos);
};

QlobberSub.prototype._add_values = function (dest, clientMap, topic)
{
    for (var clientIdAndTopicMap of clientMap)
    {
        var clientId = clientIdAndTopicMap[0];
        var topicMap = clientIdAndTopicMap[1];
        if (topic === undefined)
        {
            for (var topicAndQos of topicMap)
            {
                dest.push(
                {
                    clientId: clientId,
                    topic: topicAndQos[0],
                    qos: topicAndQos[1]
                });
            }
        }
        else
        {
            var qos = topicMap.get(topic);
            if (qos !== undefined)
            {
                dest.push(
                {
                    clientId: clientId,
                    qos: qos
                });
            }
        }
    }
};

QlobberSub.prototype._remove_value = function (clientMap, val)
{
    var topicMap = clientMap.get(val.clientId);
    if (topicMap)
    {
        topicMap.delete(val.topic);
        if (topicMap.size === 0)
        {
            clientMap.delete(val.clientId);
        }
    }
    return clientMap.size === 0;
};

QlobberSub.prototype.test_values = function (clientMap, val)
{
  var topicMap = clientMap.get(val.clientId);
  return topicMap && topicMap.has(val.topic);
};

QlobberSub.prototype.match = function (topic, ctx)
{
  return this._match([], 0, topic.split(this._separator), this._trie, ctx);
};

module.exports = QlobberSub;
