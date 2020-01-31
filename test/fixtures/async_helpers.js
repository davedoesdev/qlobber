'use strict';

exports.match = async function (matcher, topic)
{
    let r = [];

    for await (let v of matcher.match_iterP(topic))
    {
        r.push(v);
    }

    return r;
};

exports.visit = async function (matcher)
{
    let r = [];

    for await (let v of matcher.visitP())
    {
        r.push(v);
    }

    return r;
};
