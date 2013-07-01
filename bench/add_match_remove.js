/*jslint node: true */
"use strict";

var Qlobber = require('..').Qlobber,
    common = require('./common');

module.exports = function ()
{
    var matcher = new Qlobber();

    common.add_bindings(matcher);
    common.match(matcher);
    common.remove_bindings(matcher);
};

