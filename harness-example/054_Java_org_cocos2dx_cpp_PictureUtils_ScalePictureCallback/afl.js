const MODULE_WHITELIST = [
  "fuzz",
  "libBilliards.so",
];

Module.load("libandroid_runtime.so");
new ModuleMap().values().forEach(m => {
  if (!MODULE_WHITELIST.includes(m.name)) {
    Afl.addExcludedRange(m.base, m.size);
  }
});

const aflAreaPtr = Module.findExportByName("afl-frida-trace.so", "__afl_area_ptr");
const harnessSlot = Module.findExportByName("fuzz", "__omen_afl_area_ptr");
if (aflAreaPtr && harnessSlot) {
  harnessSlot.writePointer(aflAreaPtr);
}

Afl.done();
