# summary.json schema v2

Top level:

```json
{
  "schema_version": 2,
  "package": "com.example",
  "total_jni_functions": 1,
  "functions": []
}
```

Each function keeps `name`, optional `error_log`, target/priority evidence, and four stage records.
Flat fields keep the file easy to inspect and migrate.

| Stage | Status | Retry/error fields |
|---|---|---|
| `call-sequence` | `call_sequence` | `call_sequence_retries`, `call_sequence_last_error` |
| `argument-construction` | `argument_construction` | `argument_construction_retries`, `argument_construction_last_error` |
| `harness` | `harness` | `harness_retries`, `harness_last_error`, `harness_fix_applied` |
| `fuzzing` | `fuzzing` | `fuzzing_retries`, `fuzzing_last_error`, `fuzzing_fix_applied`, `fuzzing_metrics` |

Every stage also has `<field>_started_at` and `<field>_completed_at`. Common statuses are
`PENDING`, `IN_PROGRESS`, `OK`, `FAIL`, `SKIP`, and `ENVIRONMENT`.

Target fields are `target_order`, `priority_score`, `priority_tier`, `priority_rank`,
`priority_reason`, and `skip_reason`. `repair_owner` is empty or one of `call-sequence`,
`argument-construction`, `harness`, `environment`.

## Eligibility

- `call-sequence`: `target_order` is set.
- `argument-construction`: `call_sequence == OK`.
- `harness`: `argument_construction == OK`.
- `fuzzing`: `harness == OK`.

`PENDING` and `IN_PROGRESS` are pickable. `FAIL` is pickable only while its retry count is below
three and its owner is not `environment`. `ENVIRONMENT` requires `resume`; `OK` and `SKIP` are terminal.

## Legacy migration

`migrate` writes `summary.v1.json` once before replacing a legacy file. It maps old priority fields,
`phase2_callseq`, `phase3_build`, `phase3_review`, and `phase4_fuzz` without discarding metrics.
Legacy `PASS`/`APPROVED` become v2 `OK` where appropriate. A legacy successful build is evidence
that its arguments were constructed, so `argument_construction` may inherit `OK` with
`argument_construction_evidence=legacy_phase3_build`; otherwise an unproven new stage stays `PENDING`.

Migration preserves historical success only as state evidence. It does not claim that old recipes
are package-global verified Memory; those require revalidation through `memory_store.py`.
