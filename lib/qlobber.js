/**
# qlobber&nbsp;&nbsp;&nbsp;[![Build Status](https://travis-ci.org/davedoesdev/qlobber.png)](https://travis-ci.org/davedoesdev/qlobber)

Node.js globbing for amqp-like topics.

Example:

```javascript
Qlobber = require('qlobber').Qlobber;
matcher = new Qlobber();
matcher.add('foo.*', 'it matched!');
assert.deepEqual(matcher.match('foo.bar'), ['it matched!']);
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
matcher = new Qlobber({ remove_duplicates: true });
matcher.add('*.orange.*', 'Q1');
matcher.add('*.*.rabbit', 'Q2');
matcher.add('lazy.#', 'Q2');
assert.deepEqual(['quick.orange.rabbit',
                  'lazy.orange.elephant',
                  'quick.orange.fox',
                  'lazy.brown.fox',
                  'lazy.pink.rabbit',
                  'quick.brown.fox',
                  'orange',
                  'quick.orange.male.rabbit',
                  'lazy.orange.male.rabbit'].map(function (topic)
                  {
                      return matcher.match(topic).sort();
                  }),
                 [['Q1', 'Q2'],
                  ['Q1', 'Q2'],
                  ['Q1'],
                  ['Q2'],
                  ['Q2'],
                  [],
                  [],
                  [],
                  ['Q2']]);
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

// Polyfill Set if we don't have it. In the future when V8 implements Set fully,
// consider using it as the container in the trie instead of arrays.

var PSet;

if (typeof Set === 'undefined')
{
    PSet = function ()
    {
        var values = [];

        this.add = function (v)
        {
            values.push(v);
        };

        this.has = function (v)
        {
            return values.indexOf(v) >= 0;
        };
    };
}
else
{
    PSet = Set;
}

/**
Creates a new qlobber.

@constructor
@param {Object} [options] Configures the qlobber. Use the following properties:

- `{String} separator` The character to use for separating words in topics. Defaults to '.'. MQTT uses '/' as the separator, for example.

- `{String} wildcard_one` The character to use for matching exactly one word in a topic. Defaults to '*'. MQTT uses '+', for example.

- `{String} wildcard_some` The character to use for matching zero or more words in a topic. Defaults to '#'. MQTT uses '#' too.

- `{Boolean} remove_duplicates` qlobber's matching algorithm means values may be returned twice. Specify `true` to have qlobber remove duplicates from its results. Note this will incur a performance penalty. Defaults to `false`.
*/
function Qlobber (options)
{
    options = options || {};

    this._separator = options.separator || '.';
    this._wildcard_one = options.wildcard_one || '*';
    this._wildcard_some = options.wildcard_some || '#';
    this._remove_duplicates = options.remove_duplicates;
    this._trie = {};
}

Qlobber.prototype._add = function (val, i, words, sub_trie)
{
    var st, word;

    if (i === words.length)
    {
        st = sub_trie[this._separator];
        
        if (st)
        {
            st.push(val);
        }
        else
        {
            sub_trie[this._separator] = [val];
        }
        
        return;
    }

    word = words[i];
    st = sub_trie[word]
    
    if (!st)
    {
        st = sub_trie[word] = {};
    }
    
    this._add(val, i + 1, words, st);
};

Qlobber.prototype._remove = function (val, i, words, sub_trie)
{
    var st, index, word, w;

    if (i === words.length)
    {
        st = sub_trie[this._separator];

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
                delete sub_trie[this._separator];
            }
        }

        return;
    }
    
    word = words[i];
    st = sub_trie[word];

    if (!st)
    {
        return;
    }

    this._remove(val, i + 1, words, st);

    for (w in st)
    {
        return;
    }

    delete sub_trie[word];
};

Qlobber.prototype._match = function (v, i, words, sub_trie)
{
    var word, st, w, j;

    st = sub_trie[this._wildcard_some];

    if (st)
    {
        // common case: no more levels

        /*jslint forin: true */
        for (w in st)
        {
            if (w !== this._separator)
            {
                for (j = i; j < words.length; j += 1)
                {
                    v = this._match(v, j, words, st);
                }

                break;
            }
        }
        /*jslint forin: false */

        v = this._match(v, words.length, words, st);
    }

    if (i === words.length)
    {
        st = sub_trie[this._separator];

        if (st)
        {
            v = v.concat(st);
        }
    }
    else
    {
        word = words[i];

        if ((word !== this._wildcard_one) && (word !== this._wildcard_some))
        {
            st = sub_trie[word];

            if (st)
            {
                v = this._match(v, i + 1, words, st);
            }
        }

        if (word)
        {
            st = sub_trie[this._wildcard_one];

            if (st)
            {
                v = this._match(v, i + 1, words, st);
            }
        }
    }

    return v;
};

/**
Add a topic matcher to the qlobber.

Note you can match more than one value against a topic by calling `add` multiple times with the same topic and different values.

@param {String} topic The topic to match against.
@param {Any} val The value to return if the topic is matched. `undefined` is not supported.
*/
Qlobber.prototype.add = function (topic, val)
{
    this._add(val, 0, topic.split(this._separator), this._trie);
};

/**
Remove a topic matcher from the qlobber.

@param {String} topic The topic that's being matched against.
@param {Any} [val] The value that's being matched. If you don't specify `val` then all matchers for `topic` are removed.
*/
Qlobber.prototype.remove = function (topic, val)
{
    if (arguments.length === 1)
    {
        val = undefined;
    }

    this._remove(val, 0, topic.split(this._separator), this._trie);
};

/**
Match a topic.

@param {String} topic The topic to match against.
@return {Array} List of values that matched the topic. This may contain duplicates unless you configured [Qlobber](#qlobberoptions) otherwise.
*/
Qlobber.prototype.match = function (topic)
{
    var r = this._match([], 0, topic.split(this._separator), this._trie);

    if (this._remove_duplicates)
    {
        var seen = new PSet(), r2 = [], i, v;

        for (i = 0; i < r.length; i += 1)
        {
            v = r[i];

            if (!seen.has(v))
            {
                r2.push(v);
                seen.add(v);
            }
        }

        return r2;
    }

    return r;
};

/**
Reset the qlobber.

Removes all topic matchers from the qlobber.
*/
Qlobber.prototype.clear = function ()
{
    this._trie = {};
};

// for debugging
Qlobber.prototype.get_trie = function ()
{
    return this._trie;
};

exports.Qlobber = Qlobber;

