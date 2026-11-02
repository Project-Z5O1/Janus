# Standalone Verification Reference

Before Phase 4, the harness must run outside AFL on the Android device.

## Steps

1. Clean the function-specific device directory.
2. Push `fuzz`, libraries, APK, seed, and required runtime files.
3. Set executable permissions.
4. Run with absolute `LD_LIBRARY_PATH=$DEVICE_DIR/lib`.
5. Execute a small smoke input, usually `echo -n 'AAAA' | ./fuzz`.
6. Capture stdout, stderr, and logcat evidence.

## Failure Signals

Fail on missing libraries, linker errors, unresolved JNI addresses, signature mismatches, JNI exceptions during setup, crashes before the target call, or missing prerequisite artifacts.
