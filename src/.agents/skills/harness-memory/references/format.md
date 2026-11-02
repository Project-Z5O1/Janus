# Memory on-disk format

Each component directory contains immutable source artifacts, the submitted validation JSON, and
`metadata.json`:

```json
{
  "schema_version": 1,
  "kind": "argument-recipe",
  "name": "com.example.Options",
  "digest": "sha256-of-name-and-compatibility",
  "compatibility": {},
  "verification": "SYMBOL_CHECK+INDEPENDENT_BUILD+DEVICE_SMOKE_PASS",
  "source_jni": "Java_com_example_Native_read",
  "created_at": "UTC timestamp",
  "artifacts": [{"path": "construct.c", "sha256": "..."}]
}
```

`index.json` contains the same lookup metadata but not artifact contents. The artifact paths in query
results are absolute for immediate local use; the stored index uses package-relative paths so the
workspace remains movable.

Validation values:

- Call sequence: `GLOBAL_REVIEW_ACCEPT`.
- Argument recipe: exactly the combined `SYMBOL_CHECK+INDEPENDENT_BUILD+DEVICE_SMOKE_PASS` contract.

The helper intentionally does not infer verification from filenames or prose. A separate validation
JSON is required so an agent cannot accidentally promote an unfinished recipe.
