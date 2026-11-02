# Object Construction Recipes

Object recipes construct Java objects needed by target JNI parameters.

## Recipe Requirements

- Use real constructors, factories, builders, or APK-observed initialization paths.
- Document source evidence for every field or constructor argument.
- Compile and validate `construct.c` independently before using it in the harness.
- Use package/JNI/class-scoped device directories from `recipe_device_dir.py`.
- Delete local refs and check exceptions.

If a safe construction recipe cannot be proven, return a Phase 3 failure or SKIP rather than generating a crashing harness.
