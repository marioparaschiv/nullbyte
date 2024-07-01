---

<div align='center'>
   <h3>📝 nullbyte</h3>
   <p>Write empty bytes to provided memory patterns by memory patching.</p>
</div>

---

## Usage

```ts
import { patch } from 'nullbyte';

/**
 * @param {Number} pid - Process ID to attach to
 * @param {string[]} patterns - Array of memory patterns to null out
 * @param {boolean=false} matchOne - Whether to consider the patch successful if atleast one of the patterns match
 */

// Requires both patterns to be patched to consider the patch successful
patch(process.pid, ['D3 FT 55 ? 29', 'pattern2']);

// Requires only one of the patterns to be patched to consider the patch successful
patch(process.pid, ['D3 FT 55 ? 29', 'pattern2'], true);
```

## Installation
Any node package manager works.
- `npm i nullbyte`
- `pnpm i nullbyte`
- `yarn add nullbyte`

Platform specific:
- `npm i nullbyte@win32`
- `npm i nullbyte@linux`

## How it works
You may [see what nullbyte is up to behind the scenes.](https://github.com/marioparaschiv/nullbyte/blob/win32/lib/nullbyte.cpp) When `patch` is called, nullbyte will attempt to null out the bytes of the patterns you provided (will freeze the javascript thread, so nothing else will work until its done, this is more useful for patching out bytes before an app launches).

## How will I know if it was successful
If all of the conditions listed below are met, nullbyte's `patch` will return a boolean indicating whether or not the patch was successful.

## Successful conditions
nullbyte is very strict on what "success" means. nullbyte will need the following conditions to be met for the patch to be successful:
- Process ID is a number
- Patterns is an array
- Process ID is a valid running process

If `matchOne` is not specified or is `false`, the following is also required:

- All patterns are successfuly found in memory (if one doesn't get found, nullbyte will deem the patch unsuccessful, even if one pattern was patched)
