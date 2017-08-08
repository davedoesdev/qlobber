var QlobberOpts = {
    wildcard_one: '+',
    wildcard_some: '#',
    separator: '/'
};

function time(f, matcher, arg)
{
    var start = new Date();
    f(matcher, arg);
    var end = new Date();
    console.log(f.name + ':', end.getTime() - start.getTime());
}

function add_to_qlobber(matcher, add)
{
    for (var i = 0; i < 300000; i += 1)
    {
        var x = Math.floor(Math.random() * 3 + 1);
        var clientId = 'someClientId/' + i;
        add(matcher, clientId, 'a/b/c/d/' + i, 1);
        add(matcher, clientId, 'a/b/c/d/def' + i, 1);
        add(matcher, clientId, 'a/b/c/d/id' + i, 1);
        add(matcher, clientId, 'a/b/c/public/test', 1);
        add(matcher, clientId, 'a/b/c/public/all', 1);
        add(matcher, clientId, 'a/b/c/public/' + x, 1);
    }
}

function remove_from_qlobber(matcher, remove)
{
    for (var i = 0; i < 10; i += 1)
    {
        var x = Math.floor(Math.random() * 3 + 1);
        var clientId = 'someClientId/' + i;
        remove(matcher, clientId, 'a/b/c/d/' + i, 1);
        remove(matcher, clientId, 'a/b/c/d/def' + i, 1);
        remove(matcher, clientId, 'a/b/c/d/id' + i, 1);
        remove(matcher, clientId, 'a/b/c/public/test', 1);
        remove(matcher, clientId, 'a/b/c/public/all', 1);
        remove(matcher, clientId, 'a/b/c/public/' + x, 1);
    }
}

function match_client_topics(matcher, match)
{
    for (var i = 0; i < 100000; i += 1)
    {
        match(matcher, 'a/b/c/d/' + i);
    } 
}

function match_public_topics(matcher, match)
{
    for (var i = 0; i < 10; i += 1)
    {
        console.log(match(matcher, 'a/b/c/public/test').length);
    }
}

function times(QlobberClass, add, remove, match)
{
    var matcher = new QlobberClass(QlobberOpts);
    time(add_to_qlobber, matcher, add);
    time(add_to_qlobber, matcher, add);
    time(remove_from_qlobber, matcher, remove);
    time(match_client_topics, matcher, match);
    time(match_public_topics, matcher, match);
}

module.exports = times;
