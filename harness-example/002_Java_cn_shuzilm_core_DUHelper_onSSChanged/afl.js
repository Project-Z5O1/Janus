const MODULE_WHITELIST = [
  "fuzz",
  "libdu.so",
];

Afl.print("[*] Starting non-persistent FRIDA config");








let gApp = null;
let g_contextInjected = false;
const pEntryAddr = DebugSymbol.fromName("fuzz_one_input").address;
if (pEntryAddr.isNull()) {
  throw new Error("Failed to resolve fuzz_one_input");
}

const injectInterceptor = Interceptor.attach(pEntryAddr, {
  onEnter(args) {
    if (g_contextInjected) return;
    g_contextInjected = true;

    try {
      Java.perform(function () {
        const ActivityThread = Java.use('android.app.ActivityThread');
        gApp = ActivityThread.currentApplication();
        if (gApp != null) {
          const contextSym = DebugSymbol.fromName('g_frida_context');
          if (!contextSym.isNull()) {
            contextSym.address.writePointer(gApp.$handle);
          }
        }
      });
    } catch (e) {
      
      
    }

    injectInterceptor.detach();
  }
});


Module.load("libandroid_runtime.so");
new ModuleMap().values().forEach(m => {
  if (!MODULE_WHITELIST.includes(m.name)) {
    Afl.addExcludedRange(m.base, m.size);
  }
});

Afl.setInstrumentLibraries();
Afl.done();
