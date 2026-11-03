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

const hook_module = new CModule(`
  #include <string.h>
  #include <gum/gumdefs.h>

  
  #define BUF_LEN 256

  void afl_persistent_hook(GumCpuContext *regs, uint8_t *input_buf,
    uint32_t input_buf_len) {

    uint32_t length = (input_buf_len > BUF_LEN) ? BUF_LEN : input_buf_len;
    memcpy((void *)regs->x[0], input_buf, length);
    regs->x[1] = length;
  }
  `,
  {
    memcpy: Module.getExportByName(null, "memcpy")
  }
);

const pPersistentAddr = DebugSymbol.fromName("fuzz_one_input").address;
if (pPersistentAddr.isNull()) {
  throw new Error("Failed to resolve fuzz_one_input");
}

Afl.print(`[*] Persistent entry: ${formatModuleOffset(pPersistentAddr)}`);






let gApp = null;
let g_contextInjected = false;
const injectInterceptor = Interceptor.attach(pPersistentAddr, {
  onEnter(args) {
    if (g_contextInjected) return;
    g_contextInjected = true;

      try {
        Java.perform(function () {
          const contextSym = DebugSymbol.fromName('g_frida_context');
          if (contextSym.isNull()) {
            Afl.print('[!] g_frida_context symbol not found');
            return;
          }

          const ActivityThread = Java.use('android.app.ActivityThread');
          gApp = ActivityThread.currentApplication();
          if (gApp != null) {
            
            
            const env = Java.vm.getEnv();
            const globalRef = env.newGlobalRef(gApp.$handle);
            contextSym.address.writePointer(globalRef);
            Afl.print(`[+] Context injected: globalRef=${globalRef} (newGlobalRef OK)`);
          } else {
            
            contextSym.address.writePointer(ptr(1));
            Afl.print('[!] currentApplication()=null, wrote sentinel ptr(1)');
          }
        });
      } catch (e) {
        Afl.print(`[!] Frida Context injection failed: ${e}`);
        
        const contextSym = DebugSymbol.fromName('g_frida_context');
        if (!contextSym.isNull()) {
          contextSym.address.writePointer(ptr(1));
          Afl.print('[!] Fallback: wrote sentinel ptr(1)');
        }
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

Afl.setEntryPoint(pPersistentAddr);
Afl.setPersistentHook(hook_module.afl_persistent_hook);
Afl.setPersistentAddress(pPersistentAddr);
Afl.setPersistentCount(10000);
Afl.setInMemoryFuzzing();
Afl.setInstrumentLibraries();

Afl.done();
Afl.print("[*] All done!");
