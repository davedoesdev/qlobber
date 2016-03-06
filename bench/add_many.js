/*jslint node: true */
"use strict";

var Qlobber = require('..').Qlobber,
    util = require("util"),
    common = require('./common');

function Matcher()
{
    Qlobber.call(this, { separator: "/", wildcard_one: "+" });
}

util.inherits(Matcher, Qlobber);

module.exports = function ()
{
    var matcher = new Matcher(), i, j;

    for (i = 0; i < 60000; i += 1)
    {
        for (j = 0; j < 5; j += 1)
        {
            matcher.match('app/test/user/behrad/testTopic-' + j);
            matcher.add('app/test/user/behrad/testTopic-' + j, i);
        }
    }
};
