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
    return {
        topic: val.topic,
        clientMap: new Map().set(val.clientId, val.qos)
    };
};

QlobberSub.prototype._add_value = function (existing, val)
{
    existing.clientMap.set(val.clientId, val.qos);
};

QlobberSub.prototype._add_values = function (dest, existing, topic)
{
    if (topic === undefined)
    {
        for (var clientIdAndQos of existing.clientMap)
        {
            dest.push(
            {
                clientId: clientIdAndQos[0],
                topic: existing.topic,
                qos: clientIdAndQos[1]
            });
        }
    }
    else if (existing.topic === topic)
    {
        for (var clientIdAndQos of existing.clientMap)
        {
            dest.push(
            {
                clientId: clientIdAndQos[0],
                qos: clientIdAndQos[1]
            });
        }
    }
};

QlobberSub.prototype._remove_value = function (existing, val)
{
    existing.clientMap.delete(val.clientId);
    return existing.clientMap.size === 0;
};

QlobberSub.prototype.test_values = function (existing, val)
{
  return (existing.topic === val.topic) &&
         existing.clientMap.has(val.clientId);
};

QlobberSub.prototype.match = function (topic, ctx)
{
  return this._match([], 0, topic.split(this._separator), this._trie, ctx);
};

module.exports = QlobberSub;
