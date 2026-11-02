"""Stage 4: write priority.md from scores.json (sorted, ranked)."""
import json

SCORES = '/Users/syc/Person/Work/Omen/.agents/tmp/gates/scores.json'
OUT = '/Users/syc/Person/Work/Omen/target_APK/com.heytap.market/priority.md'


def esc(s):
    return s.replace('|', '\\|').replace('\n', ' ')


def main():
    sc = json.load(open(SCORES))
    for s in sc:
        s['total'] = s['c'] * 5 + s['p'] * 8 + s['d'] * 3 + s['v'] * 9
        s['tier'] = 'HIGH' if s['total'] >= 75 else ('MEDIUM' if s['total'] >= 40 else 'LOW')
    sc.sort(key=lambda x: (-x['total'], x['order']))
    lines = ['# Priority',
             '- package: com.heytap.market',
             f'- total_ranked: {len(sc)}',
             '- scoring_model: complexity*5 + pointer_ops*8 + callee_depth*3 + vuln_surface*9',
             '',
             '| priority_rank | order | jni_name | priority_score | tier | complexity | pointer_ops | callee_depth | vuln_surface | native_evidence | reason |',
             '|---:|---:|---|---:|---|---:|---:|---:|---:|---|---|']
    for rank, s in enumerate(sc, 1):
        lines.append('| {} | {} | {} | {} | {} | {} | {} | {} | {} | {} | {} |'.format(
            rank, s['order'], s['jni_name'], s['total'], s['tier'],
            s['c'], s['p'], s['d'], s['v'],
            esc(s['native_evidence']) if s['native_evidence'] else 'n/a',
            esc(s['reason'])))
    with open(OUT, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    from collections import Counter
    print('ranked:', len(sc), dict(Counter(s['tier'] for s in sc)))
    print('top5:', [(s['jni_name'], s['total']) for s in sc[:5]])


if __name__ == '__main__':
    main()
