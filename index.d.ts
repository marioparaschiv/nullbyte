declare module 'nullbyte' {
   const patch: (processId: number, patterns: string[]) => boolean;

   export = { patch };
}