/*jslint node: true */

var qlobber = require('../..').set_native(require('../../native'));

module.exports = {
    Matcher: qlobber.Qlobber.nativeString,
    check: true
};
