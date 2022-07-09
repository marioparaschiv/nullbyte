const { spawn } = require('child_process');
const scripts = {
   x64: 'run build-x64',
   ia32: 'run build-ia32'
};

function run(script) {
   console.log(`Node architecture is ${process.arch}: running "${script}"`);

   const [bin, ...args] = script.split(' ');
   spawn(bin, args, { stdio: 'inherit' });
}

if (scripts[process.arch]) {
   const npm = /^win/.test(process.platform) ? 'npm.cmd' : 'npm';
   run(`${npm} ${scripts[process.arch]}`);
} else {
   console.log('Unsupported OS, building with node-gyp regardless (! THIS MIGHT CAUSE ISSUES !).');
   run('node-gyp rebuild');
}