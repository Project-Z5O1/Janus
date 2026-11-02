# Call-Sequence Sufficiency Criteria

A Phase 2 analysis is sufficient only when the harness can prepare every target parameter and required state without guessing dangerous values.

## Required Conditions

1. Every explicit target parameter has exactly one terminal label.
2. Every `NATIVE_RETURN` chain is recursively resolved to a maximum depth of 3.
3. Every `OPAQUE_HANDLE` has a concrete factory JNI or becomes `UNRESOLVED_HANDLE` and makes the verdict insufficient.
4. Every `FIELD_READ` has a reproducible writer path or a clearly documented residual risk.
5. The prerequisite call table is ordered, structured, and directly consumable by `jni-harness`.

## Insufficient Conditions

Return `insufficient` when any parameter remains passthrough, an opaque handle lacks a factory, a required global state initializer cannot be found, or evidence is too weak to bind return values and side effects.

`UNKNOWN` is allowed only for low-confidence call arguments where passing NULL or bounded fuzz input is safe enough and the risk is documented.
