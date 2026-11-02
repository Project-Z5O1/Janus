# Input Parsing Patterns

Parse fuzz input defensively and deterministically.

## Rules

- Keep a cursor and check remaining length before every read.
- Use fixed-size reads for primitive values.
- Derive string and array lengths from bounded input bytes.
- When input is exhausted, use safe defaults such as zero length or NULL only when the parameter label allows it.
- Avoid unbounded allocation.
- Keep `BUFFER_SIZE` large enough for all planned fields while preserving AFL performance.

## Common Labels

- `FUZZ_INPUT`: read according to JNI type.
- `STRING`: bounded UTF-8 bytes passed to `NewStringUTF`.
- `SIMPLE_ARRAY`: bounded primitive array.
- `CONSTANT`: hardcoded value.
- `NATIVE_RETURN` and `OPAQUE_HANDLE`: not parsed here; values come from prerequisite calls.
