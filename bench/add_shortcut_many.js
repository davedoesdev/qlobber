/*globals options: false */
/*jslint node: true */
"use strict";

var qlobber = require('..').set_native(require('../native')),
    util = require("util");

module.exports = function ()
{
    var Matcher = options.Matcher;
    if (Matcher == qlobber.Qlobber.nativeString)
    {
        Matcher = qlobber.Qlobber.nativeNumber;
    }
    else if (Matcher == qlobber.QlobberDedup.nativeString)
    {
        Matcher = qlobber.QlobberDedup.nativeNumber;
    }

    var matcher = new Matcher(
    {
        separator: "/",
        wildcard_one: "+",
        cache_adds: true
    }), i, j;

    for (i = 0; i < 60000; i += 1)
    {
        for (j = 0; j < 5; j += 1)
        {
            if (((options.Matcher !== qlobber.Qlobber) &&
                 (options.Matcher !== qlobber.Qlobber.nativeString)) ||
                // mosca pre-dedup checks whether already added
                !matcher.test('app/test/user/behrad/testTopic-' + j, i))
            {
                matcher.add('app/test/user/behrad/testTopic-' + j, i);
            }
        }
    }
};
