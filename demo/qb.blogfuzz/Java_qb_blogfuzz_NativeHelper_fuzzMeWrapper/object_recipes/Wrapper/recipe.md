# Wrapper Object Recipe

## Class
- Full name: `qb.blogfuzz.Wrapper`
- Source: `src/java/sources/qb/blogfuzz/Wrapper.java`

## Construction Strategy
1. Accept `byte[]` constructor (Strategy 1)
- Constructor: `Wrapper(byte[] buffer)` — copies buffer internally

## Input Consumption
- Sub-parameter: `byte[] buffer` — SIMPLE_ARRAY(byte)
- Consumes remaining fuzz buffer as byte array content
- Minimum 1 byte when buffer exhausted

## Dependencies
- None (leaf type)

## Device Verification
- Status: PASS
- Device dir: namespaced via recipe_device_dir.py
