var QlobberDedup = require('../..').QlobberDedup;
var QlobberOpts = {
    wildcard_one: '+',
    wildcard_some: '#',
    separator: '/'
};
var matcher = new QlobberDedup(QlobberOpts);

var add_start = new Date();

for (var i = 0; i < 100000; i += 1)
{
    var topic = 'a/b/c/d/' + i;
    var topic2 = 'a/b/c/d/+';
    for (var j = 0; j < 50; j += 1)
    {
        var clientId = 'wxyz' + j;
        matcher.add(topic, clientId + ';' + topic + ';' + 1);
        matcher.add(topic2, clientId + ';' + topic2 + ';' + 1);
    }
}

var add_end = new Date();

console.log('add', add_end.getTime() - add_start.getTime());

var match_start = new Date();

for (var i = 0; i < 100000; i += 1)
{
    var topic = 'a/b/c/d/' + i;

    Array.from(matcher.match(topic)).map(function (m) {
        var parts = m.split(';');
        return {
            clientId: parts[0],
            topic: parts[1],
            qos: +parts[2]
        };
    });
}

var match_end = new Date();

console.log('match', match_end.getTime() - match_start.getTime());
