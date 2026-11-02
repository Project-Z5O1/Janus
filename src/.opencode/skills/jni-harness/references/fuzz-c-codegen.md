# fuzz.c Code Generation Reference

Generate `fuzz.c` from the project template and replace only the generator markers. Keep C style consistent with the repository: 2-space indent, K&R braces, standard headers first, project headers after.

## Function Resolution

Use `dlopen` only to load libraries. Resolve function addresses with `get_library_base(<lib>) + offset`. Never use `dlsym` for target or prerequisite JNI functions.

## Required Sections

- Function typedefs for target and prerequisites.
- Global function pointers.
- Library base lookup and offset binding.
- Input parsing with bounds checks.
- Object recipe calls when needed.
- Prerequisite JNI calls in table order.
- Target JNI call.
- Local-reference cleanup in reverse order.

## Coverage Reset

Include coverage reset only when prerequisites exist. Snapshot and restore the AFL bitmap around prerequisites so fuzzing observes target-only coverage. Guard bitmap access for standalone runs.
