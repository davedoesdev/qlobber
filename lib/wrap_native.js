"use strict";

const util = require('util');

module.exports = function(QlobberNative, Qlobber)
{
    class WrappedQlobberNative extends QlobberNative
    {
        constructor(options) {
            super(options);
            this.addP = util.promisify(this.add_async);
            this.removeP = util.promisify(this.remove_async);
            this.matchP = util.promisify(this.match_async);
            this.testP = util.promisify(this.test_async);
            this.clearP = util.promisify(this.clear_async);
            this._get_visitorP = util.promisify(this.get_visitor_async);
            this._visit_nextP = util.promisify(this.visit_next_async);
            this._get_restorerP = util.promisify(this.get_restorer_async);
            this._restore_nextP = util.promisify(this.restore_next_async);
            this._match_iterP = util.promisify(this.match_iter_async);
            this._match_nextP = util.promisify(this.match_next_async);
        }

        *visit()
        {
            const visitor = this.get_visitor();

            while (true)
            {
                const v = this.visit_next(visitor);
                if (v === undefined)
                {
                    break;
                }
                yield v;
            }
        }

        async *visitP()
        {
            const visitor = await this._get_visitorP();

            while (true)
            {
                const v = await this._visit_nextP(visitor);
                if (v === undefined)
                {
                    break;
                }
                yield v;
            }
        }

        *match_iter(topic, ctx) {
            const iterator = super.match_iter(topic, ctx);

            while (true)
            {
                const v = this.match_next(iterator);
                if (v === undefined)
                {
                    break;
                }
                yield v.value;
            }
        }

        async *match_iterP(topic, ctx) {
            const iterator = await this._match_iterP(topic, ctx);

            while (true)
            {
                const v = await this._match_nextP(iterator);
                if (v === undefined)
                {
                    break;
                }
                yield v.value;
            }
        }

        get_trie()
        {
            const qlobber = new Qlobber(this.options);
            const restorer = qlobber.get_restorer();

            for (let v of this.visit())
            {
                restorer(v);
            }

            return qlobber.get_trie();
        }

        get_restorer(options)
        {
            const restorer = super.get_restorer(options);
            return obj => {
                super.restore_next(restorer, obj);
            };
        }

        async get_restorerP(options)
        {
            const restorer = await this._get_restorerP(options);
            return async obj => {
                await this._restore_nextP(restorer, obj);
            };
        }
    }

    WrappedQlobberNative.is_native = true;
    WrappedQlobberNative.nonNative = Qlobber;

    return WrappedQlobberNative;
};
