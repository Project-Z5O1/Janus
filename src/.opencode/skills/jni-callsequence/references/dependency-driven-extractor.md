# Dependency-Driven Extractor

This reference describes how to build a parameter-preparation plan by closing dependencies rather than by tracing callers blindly.

## Algorithm

1. Identify the target JNI signature, declaration, call sites, and native implementation.
2. Create requirements for explicit parameters, object fields, opaque handles, global state, and native-return values.
3. Query Java, Smali, and native code for producers that can satisfy each requirement.
4. Insert accepted producers before consumers, preserving data flow and side effects.
5. Re-run closure checks until no open requirements remain or the budget is exhausted.

## Evidence Rules

Every accepted producer needs a source anchor. Return-value binding must state which call result feeds which parameter. Field and global-state writes must name the writer and the consumer. Opaque handles must be resolved through SWIG, Java handle chains, or native reinterpret evidence.

## Output Rules

The final plan must be structured: target summary, requirement table, parameter label table, prerequisite call table, object recipes, evidence anchors, and verdict.
