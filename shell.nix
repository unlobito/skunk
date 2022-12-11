{ pebbleDotNix ? null, pebbleDotNixSrc ? null }@args:

(import ./. args).devShell
