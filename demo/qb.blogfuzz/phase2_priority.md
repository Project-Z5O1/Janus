# Phase 2 Priority
- package: qb.blogfuzz
- total_ranked: 2
- scoring_model: complexity*5 + pointer_ops*8 + callee_depth*3 + vuln_surface*9

| priority_rank | order | jni_name | priority_score | tier | complexity | pointer_ops | callee_depth | vuln_surface | reason |
|--------------:|------:|----------|---------------:|------|----------:|-----------:|------------:|------------:|--------|
| 1 | 2 | Java_qb_blogfuzz_NativeHelper_fuzzMeWrapper | 26 | LOW | 2 | 2 | 0 | 0 | 3层嵌套条件分支（null/class/methodid/call返回检查），多处JNI指针跨类型转换，但无数据操作和漏洞模式 |
| 2 | 1 | Java_qb_blogfuzz_NativeHelper_fuzzMeArray | 13 | LOW | 1 | 1 | 0 | 0 | 简单null分支+JNI数组获取释放，无数据处理、无内部调用、无漏洞模式 |
