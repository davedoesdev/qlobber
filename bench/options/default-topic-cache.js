/*jslint node: true */

var qlobber = require('../..');

class Qlobber extends qlobber.Qlobber
{
    constructor()
    {
        super({ cache_topics: 1000 });
    }
}

module.exports = {
    Matcher: Qlobber,
    check: false
};
