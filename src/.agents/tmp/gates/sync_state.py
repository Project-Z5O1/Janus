"""Stage 5: sync all 988 gate results through omen_state.py target."""
import json
import subprocess
import sys

REPO = '/Users/syc/Person/Work/Omen'
HELPER = REPO + '/.agents/skills/omen-pipeline/scripts/omen_state.py'

EVID = '/Users/syc/Person/Work/Omen/.agents/tmp/gates/evidence.json'
SCORES = '/Users/syc/Person/Work/Omen/.agents/tmp/gates/scores.json'


def main():
    evid = {r['order']: r for r in json.load(open(EVID))['results']}
    scores = {s['order']: s for s in json.load(open(SCORES))}
    # order of operations: targets in order, then filtered in order
    targets = [r for r in evid.values() if r['reason'] is None]
    filtered = [r for r in evid.values() if r['reason'] is not None]

    # target_order = 1..N in original discovery order among targets
    targets.sort(key=lambda r: r['order'])
    order_map = {r['order']: i + 1 for i, r in enumerate(targets)}

    # rank map from scores (already sorted by total desc, order asc)
    sc = []
    for s in scores.values():
        s2 = dict(s)
        s2['total'] = s['c'] * 5 + s['p'] * 8 + s['d'] * 3 + s['v'] * 9
        s2['tier'] = 'HIGH' if s2['total'] >= 75 else ('MEDIUM' if s2['total'] >= 40 else 'LOW')
        sc.append(s2)
    sc.sort(key=lambda x: (-x['total'], x['order']))
    rank_map = {s['order']: i + 1 for i, s in enumerate(sc)}

    n_ok = 0
    errs = []
    # filtered first (order-independent), then targets with order/score/rank
    for r in sorted(filtered, key=lambda x: x['order']):
        reason = f"{r['reason']}: declaration at {r['decl']['file']}:{r['decl']['line']}; no verified invoke of {r['java_class']}.{r['java_method']}{r['descriptor']} in src/java or src/smali"
        cmd = [sys.executable, HELPER, 'target', '--package', 'com.heytap.market',
               '--jni-name', r['jni_name'], '--eligible', 'false',
               '--reason', reason]
        p = subprocess.run(cmd, capture_output=True, text=True, cwd=REPO)
        if p.returncode != 0:
            errs.append((r['order'], r['jni_name'], p.stderr.strip()[:200]))
        else:
            n_ok += 1
    print('filtered synced:', n_ok, 'errors:', len(errs))

    n_ok = 0
    for r in targets:
        s = scores[r['order']]
        total = s['c'] * 5 + s['p'] * 8 + s['d'] * 3 + s['v'] * 9
        tier = 'HIGH' if total >= 75 else ('MEDIUM' if total >= 40 else 'LOW')
        pr = s['reason']
        if s.get('native_evidence'):
            pr = f"{pr} [{s['native_evidence']}]"
        else:
            pr = f"no decompiled body at {r['library']}+{r['offset']}; {pr}"
        cmd = [sys.executable, HELPER, 'target', '--package', 'com.heytap.market',
               '--jni-name', r['jni_name'], '--eligible', 'true',
               '--order', str(order_map[r['order']]),
               '--score', str(total), '--tier', tier,
               '--rank', str(rank_map[r['order']]),
               '--priority-reason', pr]
        p = subprocess.run(cmd, capture_output=True, text=True, cwd=REPO)
        if p.returncode != 0:
            errs.append((r['order'], r['jni_name'], p.stderr.strip()[:200]))
        else:
            n_ok += 1
    print('targets synced:', n_ok, 'errors:', len(errs))
    for e in errs[:10]:
        print(e)


if __name__ == '__main__':
    main()
