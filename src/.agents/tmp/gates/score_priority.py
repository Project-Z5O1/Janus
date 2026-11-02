"""Stage 3: priority scoring from native pseudocode evidence only.

Scales (0..5) are derived from measurable code features; every score is
backed by counters that are echoed into the reason line. Manual overrides
list precedes the automatic scoring for hand-verified special cases.
"""
import json
import re
import sys

sys.path.insert(0, '/Users/syc/Person/Work/Omen/.agents/tmp/gates')
from native_index import find_function

EVID = '/Users/syc/Person/Work/Omen/.agents/tmp/gates/evidence.json'
OUT = '/Users/syc/Person/Work/Omen/.agents/tmp/gates/scores.json'

# Functions whose Ghidra body was not separately recovered (offset merged
# into a neighbor or landing in a gap). Verified by inspection: all sit in
# tiny gaps/tails or were absorbed into an adjacent recovered function.
NO_BODY = {
    111: 'Ghidra body not separately recovered: offset 0x6BBF8 falls inside FUN_0016bbe0 (libconscrypt_jni.c:1663, RegisterNatives clinit thunk region)',
    380: 'Ghidra body not separately recovered: offset 0x84EC8 falls inside FUN_00184e60 (libconscrypt_jni.c:16638, SSL_max_seal_overhead wrapper region)',
    422: 'Ghidra body not separately recovered: offset 0x10FF0 lies in 20-byte gap after recommendedCOutSize stub (libzstd-jni-1.5.2-2.c:37)',
    450: 'Ghidra body not separately recovered: offset 0x127B8 lies in tail of FUN_0011279c stub (libzstd-jni-1.5.2-2.c:1863)',
    454: 'Ghidra body not separately recovered: offset 0x127B0 lies in tail of FUN_0011279c stub (libzstd-jni-1.5.2-2.c:1863)',
    455: 'Ghidra body not separately recovered: offset 0x127B4 lies in tail of FUN_0011279c stub (libzstd-jni-1.5.2-2.c:1863)',
    457: 'Ghidra body not separately recovered: offset 0x12A2C lies in 16-byte gap after FUN_00112a1c (libzstd-jni-1.5.2-2.c)',
    461: 'Ghidra body not separately recovered: offset 0x12A28 lies in tail of FUN_00112a1c (libzstd-jni-1.5.2-2.c)',
    463: 'Ghidra body not separately recovered: offset 0x115AC lies in 20-byte gap after recommendedDOutSize stub (libzstd-jni-1.5.2-2.c)',
    540: 'Ghidra body not separately recovered: offset 0x216A4 falls inside FUN_001215a0 refcount/free body (libstatic-webp.c:1908)',
    550: 'Ghidra body not separately recovered: offset 0x22228 falls inside FUN_00122134 region (libstatic-webp.c)',
    573: 'Ghidra body not separately recovered: offset 0x6D02C lies in 24-byte gap of FUN_0016d014 (liboplus_conscrypt_jni.c)',
    669: 'Ghidra body not separately recovered: offset 0x73664 lies in 4-byte thunk tail of FUN_0017365c (liboplus_conscrypt_jni.c:5942)',
    670: 'Ghidra body not separately recovered: offset 0x73668 lies in 4-byte thunk tail of FUN_0017365c (liboplus_conscrypt_jni.c:5942)',
    671: 'Ghidra body not separately recovered: offset 0x7366C lies in 4-byte thunk tail of FUN_0017365c (liboplus_conscrypt_jni.c:5942)',
    672: 'Ghidra body not separately recovered: offset 0x73670 lies in 4-byte thunk tail of FUN_0017365c (liboplus_conscrypt_jni.c:5942)',
    673: 'Ghidra body not separately recovered: offset 0x73674 lies in 4-byte thunk tail of FUN_0017365c (liboplus_conscrypt_jni.c:5942)',
    778: 'Ghidra body not separately recovered: offset 0x78C0C lies in 16-byte tail of FUN_00178bfc (liboplus_conscrypt_jni.c:9797)',
    849: 'Ghidra body not separately recovered: offset 0x7C83C falls inside FUN_0017c808 wrapper region (liboplus_conscrypt_jni.c:12415)',
}

BRANCH_RE = re.compile(r'^\s*(if|else|for|while|do)\b|^\s*else\b|switch\s*\(')
CALL_RE = re.compile(r'^\s*[A-Za-z_][\w]*[\w:]*\s*\(')
INDIRECT_CALL_RE = re.compile(r'\(\*\s*\(.*\)\s*\)\s*\(')
CAST_RE = re.compile(r'\(\s*(undefined\d?\s*\*|char\s*\*|void\s*\*|long\s*\*|int\s*\*|uint\s*\*|byte\s*\*|code\s*\*|ulong\s*\*|wchar_t\s*\*|short\s*\*|ulong\s*\*|undefined\d?)\s*\)')
PTR_ARITH_RE = re.compile(r'\+\s*(0x[0-9a-fA-F]+|\d+)\s*\)|\*\([^)]*\*\)\s*[\+\-]')
MEMFN_RE = re.compile(r'\bmemcpy|memset|memmove|strcpy|strlen|strncpy|strcmp|bcopy\b')
ALLOC_RE = re.compile(r'\b(malloc|calloc|realloc|new_|_Znwm|operator_new)\b')
FREE_RE = re.compile(r'\b(free|operator_delete|_ZdlPv|ZSTD_free\w*|EVP_\w*_free|OPENSSL_cleanse)\b')
ARRAY_IDX_RE = re.compile(r'\w+\s*\[[^\]]+\]')
SIZEOF_RE = re.compile(r'\bsizeof\b|\b0x[0-9a-f]+\b')
JNIFN_RE = re.compile(
    r'\b(Get(ByteArray|Int|Long|Float|Double|Boolean|String|Object|Field|Method|StaticField|StaticMethod)'
    r'|Release(ByteArray|Int|Long|Float|Double|Boolean|String|Object)Elements'
    r'|New(ByteArray|String|Object|GlobalRef|LocalRef)'
    r'|Delete(Local|Global)Ref|ExceptionClear|ThrowNew|FindClass|GetMethodID|GetFieldID)\w*\s*\(')
LOOP_RE = re.compile(r'^\s*(for|while|do)\b')
SWITCH_RE = re.compile(r'\bswitch\s*\(')
COND_RE = re.compile(r'^\s*if\b')
GOTO_RE = re.compile(r'\bgoto\b|^\s*LAB_[0-9a-f]+')
NESTING_MARKER = re.compile(r'^  {')


def strip_comments_and_strings(body):
    out = []
    for line in body:
        l = re.sub(r'/\*.*?\*/', '', line)
        l = re.sub(r'"(?:[^"\\]|\\.)*"', '""', l)
        out.append(l)
    return out


def max_indent(body):
    m = 0
    for line in body:
        stripped = len(line) - len(line.lstrip(' '))
        m = max(m, stripped // 2)
    return m


def analyze(body):
    lines = strip_comments_and_strings(body)
    text = '\n'.join(lines)
    f = {}
    f['n_lines'] = len([l for l in lines if l.strip()])
    f['ifs'] = sum(1 for l in lines if COND_RE.match(l))
    f['loops'] = sum(1 for l in lines if LOOP_RE.match(l))
    f['switches'] = sum(1 for l in lines if SWITCH_RE.search(l))
    f['max_indent'] = max_indent(lines)
    f['gotos'] = sum(1 for l in lines if GOTO_RE.search(l))
    f['calls'] = sum(1 for l in lines if CALL_RE.match(l))
    f['indirect_calls'] = sum(1 for l in lines if INDIRECT_CALL_RE.search(l))
    f['casts'] = len(CAST_RE.findall(text))
    f['ptr_arith'] = len(PTR_ARITH_RE.findall(text))
    f['memfn'] = sum(1 for l in lines if MEMFN_RE.search(l))
    f['allocs'] = sum(1 for l in lines if ALLOC_RE.search(l))
    f['frees'] = sum(1 for l in lines if FREE_RE.search(l))
    f['array_idx'] = len(ARRAY_IDX_RE.findall(text))
    f['jni_calls'] = sum(1 for l in lines if JNIFN_RE.search(l))
    return f


def score_dimensions(f):
    """Map feature counts to 0..5 per dimension with explicit thresholds."""
    # complexity: branch/loop density + nesting + state machine feel
    branch_units = f['ifs'] + 2 * f['loops'] + 3 * f['switches']
    if branch_units == 0:
        c = 0 if f['n_lines'] < 12 else 1
    elif branch_units <= 2:
        c = 1 if f['max_indent'] <= 1 else 2
    elif branch_units <= 5:
        c = 2 if f['max_indent'] <= 2 else 3
    elif branch_units <= 10:
        c = 3 if f['max_indent'] <= 3 else 4
    elif branch_units <= 20:
        c = 4 if f['max_indent'] <= 5 else 5
    else:
        c = 5

    # pointer_ops: casts + pointer arithmetic + memcpy family + array indexing
    p_units = f['casts'] + f['ptr_arith'] + 2 * f['memfn'] + (1 if f['array_idx'] > 3 else 0)
    if p_units == 0:
        p = 0
    elif p_units <= 2:
        p = 1
    elif p_units <= 5:
        p = 2
    elif p_units <= 10:
        p = 3
    elif p_units <= 20:
        p = 4
    else:
        p = 5

    # callee_depth: direct call fan-out + indirect calls
    d_units = f['calls'] + 2 * f['indirect_calls']
    if d_units == 0:
        d = 0
    elif d_units <= 2:
        d = 1
    elif d_units <= 5:
        d = 2
    elif d_units <= 10:
        d = 3
    elif d_units <= 18:
        d = 4
    else:
        d = 5

    # vuln_surface: length/bounds inputs (array idx + memcpy), resource
    # symmetry (alloc vs free imbalance), JNI boundary traffic, deep nesting
    v_units = 0
    if f['memfn'] > 0:
        v_units += 2
    if f['array_idx'] >= 3:
        v_units += 2
    if f['allocs'] > 0 and f['frees'] == 0:
        v_units += 2          # asymmetric resource handling
    elif f['allocs'] > 0 and f['allocs'] != f['frees']:
        v_units += 1
    if f['jni_calls'] >= 4:
        v_units += 1
    if f['max_indent'] >= 4:
        v_units += 1
    if f['casts'] >= 6:
        v_units += 1
    v = min(5, v_units)
    return c, p, d, v


def main():
    d = json.load(open(EVID))
    rs = d['results']
    targets = [r for r in rs if r['reason'] is None]
    out = []
    for r in targets:
        order = r['order']
        hits = find_function(r['library'], r['offset'])
        if not hits or order in NO_BODY:
            out.append({'order': order, 'jni_name': r['jni_name'],
                        'c': 0, 'p': 0, 'd': 0, 'v': 0,
                        'native_evidence': '',
                        'reason': NO_BODY.get(order, 'decompiled body unavailable'),
                        'features': None})
            continue
        h = hits[0]
        feats = analyze(h['body'])
        c, p, dd, v = score_dimensions(feats)
        score = c * 5 + p * 8 + dd * 3 + v * 9
        anchor = f"{h['file']}:{h['line']}"
        # Build short factual reason
        bits = []
        if feats['ifs'] or feats['loops'] or feats['switches']:
            bits.append(f"{feats['ifs']} if/{feats['loops']} loop/{feats['switches']} switch, nest {feats['max_indent']}")
        else:
            bits.append('no branches')
        if feats['memfn']:
            bits.append(f"{feats['memfn']} mem* ops")
        if feats['casts']:
            bits.append(f"{feats['casts']} casts")
        if feats['array_idx'] >= 3:
            bits.append(f"{feats['array_idx']}[] idx")
        if feats['calls'] or feats['indirect_calls']:
            bits.append(f"{feats['calls']} calls/{feats['indirect_calls']} indirect")
        if feats['allocs'] or feats['frees']:
            bits.append(f"alloc {feats['allocs']}/free {feats['frees']}")
        reason = '; '.join(bits) if bits else 'trivial body'
        out.append({'order': order, 'jni_name': r['jni_name'],
                    'c': c, 'p': p, 'd': dd, 'v': v, 'score': score,
                    'native_evidence': anchor, 'reason': reason,
                    'features': feats})
    with open(OUT, 'w') as f:
        json.dump(out, f, indent=1)
    from collections import Counter
    tiers = Counter()
    for o in out:
        s = o['c']*5 + o['p']*8 + o['d']*3 + o['v']*9
        tiers['HIGH' if s >= 75 else 'MEDIUM' if s >= 40 else 'LOW'] += 1
    print('scored:', len(out), dict(tiers))


if __name__ == '__main__':
    main()
