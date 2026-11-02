# AFL Frida Script Reference

Generate `afl.js` and `afl-debug.js` for AFL++ Frida Mode.

## Requirements

- Whitelist the harness executable and all target libraries.
- Set the persistent entry point to the harness fuzz entry.
- Install the persistent hook CModule.
- Configure the same maximum input size used by `BUFFER_SIZE`.
- When coverage reset is enabled, write AFL's real bitmap pointer into the harness global `__omen_afl_area_ptr`.
- Debug scripts may enable additional logging, but release verification must use the release script first.

Call `Afl.done()` after configuration.
