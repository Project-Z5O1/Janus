# Priority Scoring Reference

Priority scoring is a conservative static ranking overlay for JNI targets that already passed Phase 1.5 filtering.

## Inputs

- `phase2_targets.md`
- `jni_list.json`
- `src/native/<abi>/<library>.c`
- Optional Java and Smali evidence

## Method

Score only targets already present in `phase2_targets.md`. Locate each native function by library and offset, then read the implementation and directly called helpers.

## Dimensions

| dimension | guidance |
| --- | --- |
| `complexity` | Branches, loops, nesting, parser/state-machine structure. |
| `pointer_ops` | Pointer arithmetic, casts, memcpy/memmove, length-index coupling, raw buffers. |
| `callee_depth` | Number and depth of internal helper calls reachable from the target. |
| `vuln_surface` | Bounds, integer, lifetime, opaque-handle, resource, and error-path patterns. |

## Output

Write `phase2_priority.md` with stable rank, score, tier, dimension scores, and concise evidence. Break ties by original `order` from `phase2_targets.md`. Copy priority fields into `summary.json` during the gate reduce.
