# Recovery and feedback rules

## Crash recovery

`start` writes `IN_PROGRESS` and a timestamp before a worker begins. A later `next` may select that
entry again. Inspect partial artifacts and logs before deciding whether to continue or regenerate them;
never interpret `IN_PROGRESS` as success. State transitions use one package lock and atomic replacement.

## Environment block

`ENVIRONMENT` means execution did not test the intended code path: missing device, NDK, Ghidra,
AFL/Frida binary, incompatible ABI, or unavailable runtime dependency. Fix the environment, then run
`resume` for the affected entry. Do not consume model retries or rewrite valid source to hide it.

## Repair invalidation

Routing to `call-sequence` resets argument construction, harness, and fuzzing. Routing to
`argument-construction` resets harness and fuzzing. Routing to `harness` resets fuzzing. If the
responsible retry budget is exhausted, downstream stages become `SKIP`; the failure remains visible.

## Terminal report

`status.all_done` means there is no executable or environment-blocked work. It does not mean every JNI
passed. Report terminal `FAIL` and `SKIP` counts separately, and include fuzzing metrics only from the
current v2 result or explicitly identified legacy migration.
