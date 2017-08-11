/*jslint node: true */
"use strict";

var util = require('util'),
    Qlobber = require('..').Qlobber;

function QlobberSub (options)
{
    Qlobber.call(this, options);
    this.subscriptionsCount = 0;
    this.subscriptionsCountPerClient = new Map();
}

util.inherits(QlobberSub, Qlobber);

QlobberSub.prototype._initial_value = function (val)
{
    this.subscriptionsCount += 1;

    var scpc = this.subscriptionsCountPerClient,
        clientId = val.clientId,
        count = scpc.get(clientId);

    if (count === undefined)
    {
        scpc.set(clientId, 1);
    }
    else
    {
        scpc.set(clientId, count + 1);
    }
        
    return {
        topic: val.topic,
        clientMap: new Map().set(clientId, val.qos)
    };
};

QlobberSub.prototype._add_value = function (existing, val)
{
    var clientMap = existing.clientMap,
        size = existing.clientMap.size,
        clientId = val.clientId;

    clientMap.set(clientId, val.qos);

    if (clientMap.size > size)
    {
        this.subscriptionsCount += 1;

        var scpc = this.subscriptionsCountPerClient,
            count = scpc.get(clientId);

        if (count === undefined)
        {
            scpc.set(clientId, 1);
        }
        else
        {
            scpc.set(clientId, count + 1);
        }
    }
};

QlobberSub.prototype._add_values = function (dest, existing, topic)
{
    var clientIdAndQos;
    if (topic === undefined)
    {
        for (clientIdAndQos of existing.clientMap)
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
        for (clientIdAndQos of existing.clientMap)
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
    var clientMap = existing.clientMap,
        size_before = clientMap.size,
        clientId = val.clientId;

    clientMap.delete(clientId);

    var size_after = clientMap.size;

    if (size_after < size_before)
    {
        this.subscriptionsCount -= 1;

        var scpc = this.subscriptionsCountPerClient,
            count = scpc.get(clientId);

        if (count === 1)
        {
            scpc.delete(clientId);
        }
        else
        {
            scpc.set(clientId, count - 1);
        }
    }

    return size_after === 0;
};

// Returns whether client is last subscriber to topic
QlobberSub.prototype.test_values = function (existing, val)
{
    var clientMap = existing.clientMap;

    return (existing.topic === val.topic) &&
           (clientMap.size === 1) &&
           clientMap.has(val.clientId);
};

QlobberSub.prototype.match = function (topic, ctx)
{
    return this._match([], 0, topic.split(this._separator), this._trie, ctx);
};

QlobberSub.prototype.clear = function ()
{
    this.subscriptionsCount = 0;
    this.subscriptionsCountPerClient.clear();
    return Qlobber.prototype.clear.call(this);
};

module.exports = QlobberSub;
