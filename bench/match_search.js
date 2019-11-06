/*globals options: false */
/*jslint node: true */
"use strict";

var qlobber = require('..'),
    MapValQlobber = require('./options/_mapval').MapValQlobber,
    common = require('./common');

var matcher_default = new qlobber.Qlobber();
common.add_bindings(matcher_default);

var matcher_default_native = new qlobber.Qlobber.nativeString();
common.add_bindings(matcher_default_native);

var matcher_dedup = new qlobber.QlobberDedup();
common.add_bindings(matcher_dedup);

var matcher_dedup_native = new qlobber.QlobberDedup.nativeString();
common.add_bindings(matcher_dedup_native);

var matcher_mapval = new MapValQlobber();
common.add_bindings(matcher_mapval);

module.exports = function ()
{
    switch (options.Matcher)
    {
        case qlobber.QlobberDedup:
            common.match_search(matcher_dedup);
            break;

        case qlobber.QlobberDedup.nativeString:
            common.match_search(matcher_dedup_native);
            break;

        case MapValQlobber:
            common.match_search(matcher_mapval);
            break;

        case qlobber.Qlobber.nativeString:
            common.match_search(matcher_default_native);
            break;

        default:
            common.match_search(matcher_default);
            break;
    }
};
