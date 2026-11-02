# Phase 2 Targets
- package: qb.blogfuzz
- total_targets: 2
- total_filtered: 0

| order | jni_name | java_class | java_method | decl_source | fuzzable_params | callsite_count | callsite_evidence |
|------:|----------|------------|-------------|-------------|-----------------|---------------:|-------------------|
| 1 | Java_qb_blogfuzz_NativeHelper_fuzzMeArray | qb.blogfuzz.NativeHelper | fuzzMeArray | java | 1 | 1 | `smali/smali_classes3/qb/blogfuzz/MainActivity.smali:89` invoke-virtual |
| 2 | Java_qb_blogfuzz_NativeHelper_fuzzMeWrapper | qb.blogfuzz.NativeHelper | fuzzMeWrapper | java | 1 | 1 | `smali/smali_classes3/qb/blogfuzz/MainActivity.smali:82` invoke-virtual |
