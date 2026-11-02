"""Stage 2: write targets.md and filtered.md from evidence.json."""
import json

EVID = '/Users/syc/Person/Work/Omen/.agents/tmp/gates/evidence.json'
OUT_DIR = '/Users/syc/Person/Work/Omen/target_APK/com.heytap.market'


def esc(s):
    return s.replace('|', '\\|').replace('\n', ' ')


def main():
    d = json.load(open(EVID))
    rs = d['results']
    targets = [r for r in rs if r['reason'] is None]
    filtered = [r for r in rs if r['reason'] is not None]
    assert len(targets) + len(filtered) == 988

    lines = ['# Targets',
             '- package: com.heytap.market',
             f'- total_targets: {len(targets)}',
             '',
             '| order | jni_name | java_class | java_method | params | decl_source | declaration_evidence | callsite_evidence |',
             '|---:|---|---|---|---|---|---|---|']
    for r in targets:
        decl = r['decl']
        cs = r['callsite']
        decl_ev = f"{decl['file']}:{decl['line']}"
        cs_ev = f"{cs['file']}:{cs['line']} {cs['kind']}"
        params = r['params'] if r['params'] else '(none)'
        lines.append('| {} | {} | {} | {} | {} | {} | {} | {} |'.format(
            r['order'], r['jni_name'], r['java_class'], r['java_method'],
            esc(params), r['decl_source'], decl_ev, esc(cs_ev)))
    with open(OUT_DIR + '/targets.md', 'w') as f:
        f.write('\n'.join(lines) + '\n')

    lines = ['# Filtered',
             '- package: com.heytap.market',
             f'- total_filtered: {len(filtered)}',
             '',
             '| order | jni_name | reason | detail |',
             '|---:|---|---|---|']
    for r in filtered:
        decl = r['decl']
        detail = (f"declaration at {decl['file']}:{decl['line']}; "
                  f"no verified invoke of {r['java_class']}.{r['java_method']}"
                  f"{r['descriptor']} in src/java or src/smali")
        lines.append('| {} | {} | {} | {} |'.format(
            r['order'], r['jni_name'], r['reason'], esc(detail)))
    with open(OUT_DIR + '/filtered.md', 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print(f'targets: {len(targets)}, filtered: {len(filtered)}')


if __name__ == '__main__':
    main()
