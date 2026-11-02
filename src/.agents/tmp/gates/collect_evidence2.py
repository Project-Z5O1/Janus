"""Stage 1 (v2): evidence collection using in-memory indexes.

Outputs evidence.json with per-function declaration/callsite evidence or
filter reason.
"""
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, '/Users/syc/Person/Work/Omen/.agents/tmp/gates')
from gate_common import PKG_ROOT, JAVA_ROOT, load_functions, descriptor_of
from grep_cache import (smali_invoke_index, java_native_index,
                        smali_method_index, java_import_index)

JAVA_ROOT_ABS = JAVA_ROOT


def extract_java_params(decl_line: str, method: str):
    """Parameter simple type names from a Java declaration line."""
    m = re.search(r'\b' + re.escape(method) + r'\s*\((.*)\)', decl_line)
    if not m:
        return None
    inner = m.group(1)
    if not inner.strip():
        return []
    parts, depth, cur = [], 0, ''
    for ch in inner:
        if ch in '<([':
            depth += 1
        elif ch in '>)]':
            depth -= 1
        if ch == ',' and depth == 0:
            parts.append(cur)
            cur = ''
        else:
            cur += ch
    if cur.strip():
        parts.append(cur)
    types = []
    for p in parts:
        p = re.sub(r'\bfinal\b', '', p).strip()
        if not p:
            continue
        toks = p.split()
        types.append(' '.join(toks[:-1]) if len(toks) >= 2 else p)
    return types


def norm_type(t: str) -> str:
    """Normalize a Java type token to the jni_list simple form."""
    t = t.strip()
    t = re.sub(r'\s+', '', t)
    # strip generic args
    depth = 0
    out = ''
    for ch in t:
        if ch == '<':
            depth += 1
        elif ch == '>':
            depth -= 1
        elif depth == 0:
            out += ch
    t = out
    # strip qualifiers like java.lang.
    for prefix in ('java.lang.', 'java.io.', 'java.nio.', 'java.util.',
                   'java.util.concurrent.atomic.', 'android.graphics.',
                   'android.view.', 'android.content.', 'java.security.'):
        if t.startswith(prefix):
            t = t[len(prefix):]
            break
    return t


def main():
    funcs = load_functions()
    sinv = smali_invoke_index()
    jnat = java_native_index()
    smeth = smali_method_index()
    jimp = java_import_index()
    print('indexes built: invokes=%d classes_with_native=%d' % (len(sinv), len(jnat)),
          file=sys.stderr)

    results = []
    stats = {'java_decl': 0, 'smali_decl': 0, 'no_decl': 0,
             'cs_smali': 0, 'cs_java': 0, 'no_cs': 0}
    for idx, fn in enumerate(funcs, 1):
        jc, jm = fn['java_class'], fn['java_method']
        desc = descriptor_of(fn['params'], fn['return_type'], jc)
        want_params = ([p.strip() for p in fn['params'].split(',')]
                       if fn['params'].strip() else [])

        # ---- declaration ----
        decl = None
        decl_source = None
        java_soft = []
        for rel, ln, txt in jnat.get(jc, []):
            if not re.search(r'\b' + re.escape(jm) + r'\s*\(', txt):
                continue
            java_soft.append({'file': rel, 'line': ln, 'text': txt})
        for cand in java_soft:
            got = extract_java_params(cand['text'], jm)
            if got is None:
                continue
            got_n = [norm_type(g) for g in got]
            if got_n == want_params:
                decl = cand
                decl_source = 'java'
                break
        if decl is None and len(java_soft) == 1:
            # single native method with this name in class: params compare
            # failed only due to source-level generic/wildcard spelling
            got = extract_java_params(java_soft[0]['text'], jm)
            if got is not None:
                decl = java_soft[0]
                decl_source = 'java'
        if decl is None:
            # smali fallback (descriptor-exact)
            for m in smeth.get(jc, []):
                if (m['native'] and m['name'] == jm and m['desc'] == desc):
                    decl = {'file': m['file'], 'line': m['line'], 'text': m['text']}
                    decl_source = 'smali_fallback'
                    break
        smali_soft = None
        if decl is None:
            for m in smeth.get(jc, []):
                if m['native'] and m['name'] == jm:
                    smali_soft = {'file': m['file'], 'line': m['line'],
                                  'text': m['text'], 'desc': m['desc']}
                    break
        if decl is None:
            stats['no_decl'] += 1
            results.append({
                'order': idx, 'jni_name': fn['jni_name'], 'java_class': jc,
                'java_method': jm, 'params': fn['params'],
                'return_type': fn['return_type'], 'descriptor': desc,
                'decl': None, 'decl_source': None, 'callsite': None,
                'reason': 'FILTERED_NON_JAVA_NATIVE',
                'java_soft': java_soft, 'smali_soft': smali_soft,
                'library': fn['library'], 'offset': fn['offset']})
            continue
        stats['java_decl' if decl_source == 'java' else 'smali_decl'] += 1

        # ---- callsite ----
        owner = 'L' + jc.replace('.', '/') + ';'
        key = owner + '->' + jm + desc
        hits = sinv.get(key, [])
        callsite = None
        if hits:
            rel, ln, kind, txt = hits[0]
            callsite = {'source': 'smali', 'file': rel, 'line': ln,
                        'kind': kind, 'text': txt, 'n_hits': len(hits)}
            stats['cs_smali'] += 1
        else:
            # Java-side search: own file + importers, resolved reference
            cs = java_callsite(jc, jm, want_params, jimp)
            if cs:
                callsite = cs
                stats['cs_java'] += 1
        if callsite is None:
            stats['no_cs'] += 1
        results.append({
            'order': idx, 'jni_name': fn['jni_name'], 'java_class': jc,
            'java_method': jm, 'params': fn['params'],
            'return_type': fn['return_type'], 'descriptor': desc,
            'decl': decl, 'decl_source': decl_source, 'callsite': callsite,
            'reason': None if callsite else 'FILTERED_NO_JAVA_CALLSITE',
            'java_soft': java_soft if decl_source != 'java' else None,
            'smali_soft': smali_soft,
            'library': fn['library'], 'offset': fn['offset']})

    with open('/Users/syc/Person/Work/Omen/.agents/tmp/gates/evidence.json', 'w') as f:
        json.dump({'stats': stats, 'results': results}, f, indent=1)
    print(json.dumps(stats))


def java_callsite(jc, jm, want_params, jimp):
    """Find a resolved Java invocation of jc.jm.

    Candidate files: the class's own file (unqualified calls) plus files
    importing jc (qualified calls Class.method or instance.method where the
    receiver's declared type comes from the import). We require either an
    own-file call or a receiver expression whose type we can tie to jc.
    """
    simple = jc.rsplit('.', 1)[1]
    own_rel = 'src/java/sources/' + jc.replace('.', '/') + '.java'
    candidates = {own_rel}
    candidates |= jimp.get(jc, set())
    n_want = len(want_params)
    best = None
    fallback = None
    for rel in candidates:
        path = JAVA_ROOT_ABS / rel[len('src/java/sources/'):]
        try:
            lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
        except OSError:
            continue
        own = rel == own_rel
        # receiver map: declared fields/locals of type simple (very crude,
        # only used to accept `field.method(` in importing files)
        for i, line in enumerate(lines, 1):
            ls = line.strip()
            if jm + '(' not in ls and jm + ' (' not in ls:
                continue
            if ls.startswith('//') or ls.startswith('*') or ls.startswith('/*'):
                continue
            # exclude the declaration line
            if re.search(r'\bnative\b', ls) and re.search(
                    r'\b' + re.escape(jm) + r'\s*\(', ls) and ls.endswith(';'):
                continue
            qual = re.search(r'\b' + re.escape(simple) + r'\s*\.\s*'
                             + re.escape(jm) + r'\s*\(', ls)
            unqual = re.search(r'(?:^|[^.\w])' + re.escape(jm) + r'\s*\(', ls)
            if qual or (own and unqual):
                m = re.search(re.escape(jm) + r'\s*\((.*)\)', ls)
                args = None
                if m:
                    inner = m.group(1)
                    depth, commas = 0, (0 if inner.strip() == '' else 1)
                    for ch in inner:
                        if ch in '([':
                            depth += 1
                        elif ch in ')]':
                            depth -= 1
                        elif ch == ',' and depth == 0:
                            commas += 1
                    args = commas
                rec = {'source': 'java', 'file': rel, 'line': i,
                       'kind': 'java_call', 'text': ls[:160],
                       'args': args, 'n_hits': 1}
                if args == n_want:
                    if best is None or (own and not best.get('own')):
                        rec['own'] = own
                        best = rec
                elif fallback is None:
                    fallback = rec
    return best or fallback


if __name__ == '__main__':
    main()
