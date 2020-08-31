/*globals options: false */
/*jslint node: true */
"use strict";

var qlobber = require('..').set_native(require('../native')),
    MapValQlobber = require('./options/_mapval').MapValQlobber,
    common = require('./common');

var matcher_options = {
    separator: "/",
    wildcard_one: "+",
    cache_adds: true
};

function add_bindings(matcher)
{
    var i, j;
    for (i = 0; i < 60000; i += 1)
    {
        for (j = 0; j < 100; j += 1)
        {
            matcher.add('app/test/user/behrad/testTopic-' + j, i);
        }
        matcher.add('app/test/user/behrad/+', i);
    }
}

var matcher_default = new qlobber.Qlobber(matcher_options);
add_bindings(matcher_default);

var matcher_default_native = new qlobber.Qlobber.nativeNumber(matcher_options);
add_bindings(matcher_default_native);

var matcher_dedup = new qlobber.QlobberDedup(matcher_options);
add_bindings(matcher_dedup);

var matcher_dedup_native = new qlobber.QlobberDedup.nativeNumber(matcher_options);
add_bindings(matcher_dedup_native);

var matcher_mapval = new MapValQlobber(matcher_options);
add_bindings(matcher_mapval);

module.exports = function ()
{
    var j;

	for (j = 0; j < 100; j += 1)
	{
		switch (options.Matcher)
		{
			case qlobber.QlobberDedup:
				matcher_dedup.match('app/test/user/behrad/testTopic-' + j);
				break;

            case qlobber.QlobberDedup.nativeString:
				matcher_dedup_native.match('app/test/user/behrad/testTopic-' + j);
				break;

			case MapValQlobber:
				matcher_mapval.match('app/test/user/behrad/testTopic-' + j);
				break;

            case qlobber.Qlobber.nativeString:
				common.remove_duplicates(matcher_default_native.match('app/test/user/behrad/testTopic-' + j));
				break;

			default:
				common.remove_duplicates(matcher_default.match('app/test/user/behrad/testTopic-' + j));
				break;
		}
	}
};
