<div align='center'>
   <h2>
      <b>nullbyte</b>
   </h2>
   <div>
      Write empty bytes to provided memory patterns by memory patching.
   </div>
</div>

### Usage

ESM
```ts
import { patch } from 'nullbyte';

patch(process.pid, ['D3 FT 55 ? 29', 'pattern2'])
```

CJS
```ts
const { patch } = require('nullbyte');

patch(process.pid, ['D3 FT 55 ? 29', 'pattern2'])
```

### How it works
You may [see what nullbyte is up to behind the scenes.](https://github.com/localip/nullbyte/blob/win32/lib/nullbyte.cpp) When `patch` is called, nullbyte will attempt to null out the bytes of the patterns you provided (will freeze the javascript thread, so nothing else will work until its done, this is more useful for patching out bytes before an app launches).

### How will I know if it was successful
If all of the conditions listed below are met, nullbyte's `patch` will return a boolean indicating whether or not the patch was successful.

### Successful conditions
nullbyte is very strict on what "success" means. nullbyte will need the following conditions to be met for the patch to be successful:
- Process ID is a number
- Patterns is an array
- Process ID is a valid running process
- All patterns are successfuly found in memory (if one doesn't get found, nullbyte will deem the patch unsuccessful, even if one pattern was patched)
