var QlobberSub = require('../qlobber-sub');
var QlobberOpts = {
    wildcard_one: '+',
    wildcard_some: '#',
    separator: '/'
};
var matcher = new QlobberSub(QlobberOpts);

var add_start = new Date();

for (var i = 0; i < 100000; i += 1)
{
    var topic = 'a/b/c/d/' + i;
    var topic2 = 'a/b/c/d/+';
    for (var j = 0; j < 50; j += 1)
    {
        var clientId = 'wxyz' + j;
        matcher.add(topic, { clientId: clientId, topic: topic, qos: 1 });
        matcher.add(topic2, { clientId: clientId, topic: topic2, qos: 1 });
    }
}

var add_end = new Date();

console.log('add', add_end.getTime() - add_start.getTime());

var match_start = new Date();

for (var i = 0; i < 100000; i += 1)
{
    var topic = 'a/b/c/d/' + i;
    matcher.match(topic);
}

var match_end = new Date();

console.log('match', match_end.getTime() - match_start.getTime());
