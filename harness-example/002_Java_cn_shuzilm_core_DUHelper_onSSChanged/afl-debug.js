Afl.print(`[*] Starting FRIDA config for PID: ${Process.id}`);

const MODULE_WHITELIST = [
  "fuzz",
  "fuzz-debug",
  "libdu.so",
];

function asPointer(value) {
  if (value === null || value === undefined) {
    return null;
  }

  try {
    return ptr(value);
  } catch (error) {
    return null;
  }
}

function formatModuleOffset(value) {
  const address = asPointer(value);

  if (address === null) {
    return "n/a";
  }

  const module = Process.findModuleByAddress(address);

  if (module === null) {
    return address.toString();
  }

  return `${module.name}+0x${address.sub(module.base).toString(16)}`;
}

function formatSymbol(value) {
  const address = asPointer(value);

  if (address === null) {
    return "unknown";
  }

  try {
    return DebugSymbol.fromAddress(address).toString();
  } catch (error) {
    return "unknown";
  }
}

Process.setExceptionHandler(details => {
  const faultAddress = details.address ?? null;
  const pcAddress = details.context?.pc ?? null;

  Afl.print(
    `[EXCEPTION] type=${details.type} fault=${formatModuleOffset(faultAddress)} ` +
    `pc=${formatModuleOffset(pcAddress)} symbol=${formatSymbol(pcAddress)}`
  );

  return false;
});

Afl.print("[*] Starting non-persistent FRIDA config");


let gApp = null;
let g_contextInjected = false;
const pEntryAddr = DebugSymbol.fromName("fuzz_one_input").address;
if (pEntryAddr.isNull()) {
  throw new Error("Failed to resolve fuzz_one_input");
}

Afl.print(`[*] Entry point: ${formatModuleOffset(pEntryAddr)}`);

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
            Afl.print(`[+] Context injected: handle=${gApp.$handle} (writePointer OK)`);
          } else {
            Afl.print('[!] g_frida_context symbol not found in fuzz binary');
          }
        } else {
          Afl.print('[!] currentApplication() returned null, Context NOT injected');
        }
      });
    } catch (e) {
      Afl.print(`[!] Frida Context injection failed: ${e}`);
    }

    injectInterceptor.detach();
    Afl.print('[*] Context injector detached (one-time only)');
  }
});


Module.load("libandroid_runtime.so");
new ModuleMap().values().forEach(m => {
  if (!MODULE_WHITELIST.includes(m.name)) {
    Afl.print(`Exclude: ${m.base}-${m.base.add(m.size)} ${m.name}`);
    Afl.addExcludedRange(m.base, m.size);
  }
});

Afl.setInstrumentLibraries();
Afl.done();
Afl.print("[*] All done!");
