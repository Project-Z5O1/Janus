const MODULE_WHITELIST = [
  "fuzz",
  "libdu.so",
];

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

Module.load("libandroid_runtime.so");
new ModuleMap().values().forEach(m => {
  if (!MODULE_WHITELIST.includes(m.name)) {
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
