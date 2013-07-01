/*jslint node: true */
"use strict";

var Qlobber = require('..').Qlobber,
    common = require('./common');

var matcher = new Qlobber();
common.add_bindings(matcher);

module.exports = function ()
{
    common.match(matcher);
};

