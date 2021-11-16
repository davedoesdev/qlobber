Adds async and worker thread support to [qlobber](https://github.com/davedoesdev/qlobber).

Documentation and tests are part of qlobber. This module isn't meant to be
used standalone. Add it to qlobber like this:

```javascript
require('qlobber').set_native(require('qlobber-native'));
```

Note that [Boost](https://www.boost.org/) is required for building this module,
including the `boost_context` runtime library.
