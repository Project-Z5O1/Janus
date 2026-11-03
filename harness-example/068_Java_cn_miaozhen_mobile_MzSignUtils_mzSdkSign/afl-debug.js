Afl.print(`[*] Starting FRIDA config for PID: ${Process.id}`);

const MODULE_WHITELIST = [
  "fuzz",
  "fuzz-debug",
  
  
  "MZSignature",
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

  
  
  #define BUF_LEN 1560

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

Module.load("libandroid_runtime.so");
new ModuleMap().values().forEach(m => {
  if (!MODULE_WHITELIST.includes(m.name)) {
    Afl.print(`Exclude: ${m.base}-${m.base.add(m.size)} ${m.name}`);
    Afl.addExcludedRange(m.base, m.size);
  }
});





const aflAreaPtr = Module.findExportByName("afl-frida-trace.so", "__afl_area_ptr");
const harnessSlot = Module.findExportByName("fuzz", "__omen_afl_area_ptr");
if (aflAreaPtr && harnessSlot) {
  harnessSlot.writePointer(aflAreaPtr);
  Afl.print(`[*] Coverage reset: afl_area_ptr=${aflAreaPtr} → harness slot=${harnessSlot}`);
} else {
  Afl.print(`[!] Coverage reset: skipped (aflAreaPtr=${aflAreaPtr}, harnessSlot=${harnessSlot})`);
}




Afl.setPersistentCount(1000);

Afl.setEntryPoint(pPersistentAddr);
Afl.setPersistentHook(hook_module.afl_persistent_hook);
Afl.setPersistentAddress(pPersistentAddr);
Afl.setInMemoryFuzzing();
Afl.setInstrumentLibraries();

Afl.done();
Afl.print("[*] All done!");
