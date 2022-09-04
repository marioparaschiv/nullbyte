declare module 'nullbyte' {
   /**
    * @param {Number} pid - Process ID to attach to
    * @param {string[]} patterns - Array of memory patterns to null out
    * @param {boolean=false} matchOne - Whether to consider the patch successful if atleast one of the patterns match
    */
   const patch: (processId: number, patterns: string[], matchOne: boolean = false) => boolean;

   export = { patch };
}