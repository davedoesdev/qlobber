/**
# qlobber&nbsp;&nbsp;&nbsp;[![Build Status](https://travis-ci.org/davedoesdev/qlobber.png)](https://travis-ci.org/davedoesdev/qlobber)

Node.js globbing for amqp-like topics.

Example:

```javascript
Qlobber = require('qlobber').Qlobber;
matcher = new Qlobber();
matcher.add('foo.*', 'it matched!', function ()
{
    matcher.match('foo.bar', function (err, vals)
    {
        assert.deepEqual(vals, ['it matched!']);
    });
});
```

The API is described [here](#tableofcontents).

qlobber is implemented using a trie, as described in the RabbitMQ blog posts [here](http://www.rabbitmq.com/blog/2010/09/14/very-fast-and-scalable-topic-routing-part-1/) and [here](http://www.rabbitmq.com/blog/2011/03/28/very-fast-and-scalable-topic-routing-part-2/).

## Installation

```shell
npm install qlobber
```

## Another Example

A more advanced example using topics from the [RabbitMQ topic tutorial](http://www.rabbitmq.com/tutorials/tutorial-five-python.html):

```javascript
async.parallel(
    [matcher.add.bind(matcher, '*.orange.*', 'Q1'),
     matcher.add.bind(matcher, '*.*.rabbit', 'Q2'),
     matcher.add.bind(matcher, 'lazy.#', 'Q2')],
    async.mapSeries.bind(async,
        ['quick.orange.rabbit',
         'lazy.orange.elephant',
         'quick.orange.fox',
         'lazy.brown.fox',
         'lazy.pink.rabbit',
         'quick.brown.fox',
         'orange',
         'quick.orange.male.rabbit',
         'lazy.orange.male.rabbit'],
        matcher.match,
        function (err, vals)
        {
            assert.deepEqual(vals,
                [['Q1', 'Q2'],
                 ['Q1', 'Q2'],
                 ['Q1'],
                 ['Q2'],
                 ['Q2'],
                 [],
                 [],
                 [],
                 ['Q2']]);
        }));
```

## Licence

[MIT](LICENCE)

## Tests

qlobber passes the [RabbitMQ topic tests](https://github.com/rabbitmq/rabbitmq-server/blob/master/src/rabbit_tests.erl) (I converted them from Erlang to Javascript).

To run the tests:

```javascript
grunt test
```

## Lint

```javascript
grunt lint
```

# API
*/

/*jslint node: true */
"use strict";

var async = require('async');

/**
Creates a new qlobber.

@constructor
@param {Object} [options] Configures the globber. Use the following properties:

- `{String} separator` The character to use for separating words in topics. Defaults to '.'. MQTT uses '/' as the separator, for example.

- `{String} wildcard_one` The character to use for matching exactly one word in a topic. Defaults to '*'. MQTT uses '+', for example.

- `{String} wildcard_some` The character to use for matching zero or more words in a topic. Defaults to '#'. MQTT uses '#' too.

- `{String | false} compare` The function to use for sorting matches in order to remove duplicates. Defaults to lexicographical string compare. Specify `false` to turn off duplicate removal. If you store values other than strings in qlobber, pass in your own compare function.
*/
function Qlobber (options)
{
    options = options || {};

    var QlobberObject = this,
        separator = options.separator || '.',
        wildcard_one = options.wildcard_one || '*',
        wildcard_some = options.wildcard_some || '#',
        trie = {},
        
    add = function (val, i, words, sub_trie, cb)
    {
        var st, word;

        if (i === words.length)
        {
            st = sub_trie[separator] = (sub_trie[separator] || []);
            st.push(val);
            process.nextTick(cb);
            return;
        }

        word = words[i];

        st = sub_trie[word] = (sub_trie[word] || {});

        process.nextTick(add.bind(this, val, i + 1, words, st, cb));
    },

    remove = function (val, i, words, sub_trie, cb)
    {
        var word, st, index;

        if (i === words.length)
        {
            st = sub_trie[separator];

            if (st)
            {
                if (val === undefined)
                {
                    st = [];
                }
                else
                {
                    index = st.lastIndexOf(val);

                    if (index >= 0)
                    {
                        st.splice(index, 1);
                    }
                }

                if (st.length === 0)
                {
                    delete sub_trie[separator];
                }
            }

            process.nextTick(cb);
            return;
        }
        
        word = words[i];
        st = sub_trie[word];

        if (!st)
        {
            process.nextTick(cb);
            return;
        }

        process.nextTick(remove.bind(this, val, i + 1, words, st, function ()
        {
            var w, st = sub_trie[word];

            if (!st)
            {
                process.nextTick(cb);
                return;
            }

            for (w in st)
            {
                if (st.hasOwnProperty(w))
                {
                    process.nextTick(cb);
                    return;
                }
            }

            delete sub_trie[word];
            process.nextTick(cb);
        }));
    },
    
    match = function (i, words, sub_trie, cb)
    {
        var word, st, sts = [], w, j, done = cb;

        st = sub_trie[wildcard_some];

        if (st)
        {
            // common case: no more levels

            /*jslint forin: true */
            for (w in st)
            {
                if (w !== separator)
                {
                    for (j = i; j < words.length; j += 1)
                    {
                        sts.push({ i: j, st: st });
                    }

                    break;
                }
            }
            /*jslint forin: false */

            sts.push({ i: words.length, st: st });
        }

        if (i === words.length)
        {
            st = sub_trie[separator];

            if (st)
            {
                done = function (st, err, r)
                {
                    cb(err, st.concat(r));
                }.bind(this, st);
            }
        }
        else
        {
            word = words[i];

            if ((word !== wildcard_one) && (word !== wildcard_some))
            {
                st = sub_trie[word];

                if (st)
                {
                    sts.push({ i: i + 1, st: st });
                }
            }

            if (word)
            {
                st = sub_trie[wildcard_one];

                if (st)
                {
                    sts.push({ i: i + 1, st: st });
                }
            }
        }

        process.nextTick(async.concat.bind(this, sts, function (st, cb)
        {
            match(st.i, words, st.st, cb);
        }, function (err, r)
        {
            process.nextTick(done.bind(this, err, r));
        }));
    };

/**
Add a topic matcher to the qlobber.

Note you can match more than one value against a topic by calling `add` multiple times with the same topic and different values.

@param {String} topic The topic to match against.
@param {Any} val The value to return if the topic is matched. `undefined` is not supported.
@param {Function} cb Called when the matcher has been added.
*/
    QlobberObject.add = function (topic, val, cb)
    {
        add(val, 0, topic.split(separator), trie, cb);
    };

/**
Remove a topic matcher from the qlobber.

@param {String} topic The topic that's being matched against.
@param {Any} [val] The value that's being matched. If you don't specify `val` then all matchers for `topic` are removed.
@param {Function} cb Called when the matcher has been removed.
*/
    QlobberObject.remove = function (topic, val, cb)
    {
        if (arguments.length === 2)
        {
            remove(undefined, 0, topic.split(separator), trie, val);
        }
        else
        {
            remove(val, 0, topic.split(separator), trie, cb);
        }
    };

/**
Match a topic.

@param {String} topic The topic to match against.
@param {Function} cb Called with two arguments when the match has completed:

- `{Any} err` `null` or an error, if one occurred.
- `{Array} vals` List of values that matched the topic. `vals` will be sorted and have duplicates removed unless you configured [Qlobber](#qlobberoptions) otherwise.
*/
    QlobberObject.match = function (topic, cb)
    {
        /*jslint unparam: true */
        match(0, topic.split(separator), trie, function (err, r)
        {
            if (options.compare !== false)
            {
                r = r.sort(options.compare).reduce(function (prev, cur)
                {
                    if (prev[prev.length - 1] !== cur)
                    {
                        prev.push(cur);
                    }

                    return prev;
                }, [undefined]);

                r.shift();
            }

            cb(null, r);
        });
        /*jslint unparam: false */
    };

/**
Reset the qlobber.

Removes all topic matchers from the qlobber.

@param {Function} Called when the qlobber has been resets
*/
    QlobberObject.clear = function (cb)
    {
        trie = {};
        cb();
    };

    // for debugging

    QlobberObject.get_trie = function ()
    {
        return trie;
    };
}

exports.Qlobber = Qlobber;

