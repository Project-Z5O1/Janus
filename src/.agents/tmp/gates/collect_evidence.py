"""Stage 1: extract declaration + callsite evidence for all 988 bindings.

Outputs evidence.json with per-function:
  decl: {source: java|smali_fallback, file, line, text}
  callsite: {source: java|smali, file, line, text, kind}
or filter reason.
"""
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, '/Users/syc/Person/Work/Omen/.agents/tmp/gates')
from gate_common import (PKG_ROOT, JAVA_ROOT, SMALI_ROOTS, load_functions,
                         smali_path_for, descriptor_of)

JAVA_DECL_RE = re.compile(
    r'(?P<text>^\s*(?:public|private|protected|static|final|synchronized|abstract|'
    r'volatile|transient|strictfp|default|\s)*'
    r'(?:public\s+|private\s+|protected\s+|static\s+|final\s+|synchronized\s+|native\s+)+'
    r'native\s+[\w\[\].<>?,\s]+\b(?P<name>[A-Za-z_$][A-Za-z0-9_$]*)\s*\()',
    re.M)

SMALI_METHOD_RE = re.compile(r'^\.method\s+(?P<access>.*)$')
SMALI_INVOKE_RE = re.compile(
    r'^\s*(?P<kind>invoke-(?:virtual|static|direct|super|interface|range|polymorphic|custom))'
    r'\s*(?:/range)?\s*\{[^}]*\},\s*(?P<owner>L[\w/$]+;)->(?P<name>[A-Za-z0-9_$<>\[\];./-]+)'
    r'(?P<desc>\([^)]*\).+)$')

ESCAPES = {'t': '\t', 'b': '\b', 'n': '\n', 'r': '\r', 'f': '\f',
           '0': '\0', '\\': '\\', '"': '"', "'": "'"}


def unescape_java(s: str) -> str:
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == '\\' and i + 1 < len(s):
            nxt = s[i + 1]
            if nxt == 'u' and i + 5 < len(s):
                try:
                    out.append(chr(int(s[i + 2:i + 6], 16)))
                    i += 6
                    continue
                except ValueError:
                    pass
            out.append(ESCAPES.get(nxt, nxt))
            i += 2
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def find_java_native_decls(java_class: str, java_method: str):
    """Return [(file, line, text)] for native method declarations in the
    class's own .java file (plus nested-class matches)."""
    rel = java_class.replace('.', '/') + '.java'
    path = JAVA_ROOT / rel
    results = []
    if not path.exists():
        return None, []
    text = path.read_text(encoding='utf-8', errors='replace')
    for i, line in enumerate(text.splitlines(), 1):
        ls = line.strip()
        if 'native' not in ls or java_method not in ls:
            continue
        # must look like a declaration: contains 'native' keyword token and
        # the method name followed by '('
        if re.search(r'\bnative\b', ls) and re.search(
                r'\b' + re.escape(java_method) + r'\s*\(', ls):
            results.append((rel, i, ls))
    return path, results


def parse_smali_methods(path: Path):
    """Return list of dicts for .method entries in a smali file."""
    out = []
    try:
        lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
    except OSError:
        return out
    for i, line in enumerate(lines, 1):
        if line.startswith('.method'):
            m = re.match(r'^\.method\s+(?P<acc>.+?)\s+(?P<sig>[\w$<>]+\([^)]*\).+)$',
                         line.strip())
            if m:
                name, desc = split_smali_sig(m.group('sig'))
                out.append({'line': i, 'text': line.strip(),
                            'access': m.group('acc'), 'name': name, 'desc': desc})
    return out


def split_smali_sig(sig: str):
    i = sig.find('(')
    if i < 0:
        return sig, ''
    return sig[:i], sig[i:]


def find_smali_native_decl(java_class: str, java_method: str, desc: str):
    path = smali_path_for(java_class)
    if path is None:
        return None
    for m in parse_smali_methods(path):
        if m['name'] == java_method and m['desc'] == desc and 'native' in m['access']:
            rel = str(path.relative_to(PKG_ROOT / 'src'))
            return {'file': rel, 'line': m['line'], 'text': m['text']}
    # second pass: name match without descriptor (needs manual review)
    return None


def find_smali_native_decl_soft(java_class: str, java_method: str):
    """Name-only native declaration match, for diagnostics."""
    path = smali_path_for(java_class)
    if path is None:
        return None
    for m in parse_smali_methods(path):
        if m['name'] == java_method and 'native' in m['access']:
            rel = str(path.relative_to(PKG_ROOT / 'src'))
            return {'file': rel, 'line': m['line'], 'text': m['text'], 'desc': m['desc']}
    return None


def find_java_callsites(java_class: str, java_method: str, desc: str,
                        param_count: int, java_file: Path):
    """Search the app's Java sources for resolved invocations of
    java_class.java_method. Returns list of (relfile, line, text).

    Strategy: find candidate files importing or referencing the class,
    then look for call expressions `.java_method(`; resolve enclosing
    class by package/import heuristics and reject false short-name hits
    by checking the receiver type where visible.
    """
    simple = java_class.rsplit('.', 1)[1]
    hits = []
    if java_file is None:
        return hits
    # Restrict to the class's own file plus files that import the class.
    candidates = [(java_file, str(java_file.relative_to(JAVA_ROOT)))]
    importers = grep_importers(java_class)
    for p in importers:
        candidates.append((p, str(p.relative_to(JAVA_ROOT))))
    for path, rel in candidates:
        try:
            text = path.read_text(encoding='utf-8', errors='replace')
        except OSError:
            continue
        for i, line in enumerate(text.splitlines(), 1):
            ls = line.strip()
            if not re.search(re.escape(java_method) + r'\s*\(', ls):
                continue
            # unqualified call inside the class's own file (this/super implicit)
            own = path == java_file
            qualified = re.search(
                r'\b' + re.escape(simple) + r'\s*\.\s*' + re.escape(java_method) + r'\s*\(',
                ls)
            thiscall = re.search(r'(?:^|[^.\w])' + re.escape(java_method) + r'\s*\(',
                                 ls)
            if qualified or (own and thiscall):
                # crude arg-count check: count top-level commas in the call
                m = re.search(re.escape(java_method) + r'\s*\((.*)\)', ls)
                args = None
                if m:
                    inner = m.group(1)
                    depth = 0
                    commas = 0 if inner.strip() == '' else 1
                    for ch in inner:
                        if ch in '([':
                            depth += 1
                        elif ch in ')]':
                            depth -= 1
                        elif ch == ',' and depth == 0:
                            commas += 1
                    args = commas
                hits.append({'file': 'src/java/sources/' + rel, 'line': i,
                             'text': ls[:160], 'args': args, 'own': own,
                             'qualified': bool(qualified)})
    return hits


_IMPORT_CACHE = {}


def grep_importers(java_class):
    """Files whose imports contain the class, cached per class."""
    if java_class in _IMPORT_CACHE:
        return _IMPORT_CACHE[java_class]
    import subprocess
    escaped = java_class.replace('.', r'\.')
    cmd = ['grep', '-rl', '--include=*.java', '-E',
           'import\s+' + escaped + r'\s*;', str(JAVA_ROOT)]
    try:
        out = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        files = [Path(x) for x in out.stdout.splitlines() if x]
    except Exception:
        files = []
    _IMPORT_CACHE[java_class] = files
    return files


def find_smali_callsites(java_class: str, java_method: str, desc: str):
    """grep smali tree for invoke- of owner->name+desc."""
    import subprocess
    owner = 'L' + java_class.replace('.', '/') + ';'
    needle = owner + '->' + java_method + desc
    cmd = ['grep', '-rn', '--include=*.smali', '-F', needle, str(PKG_ROOT / 'src' / 'smali')]
    out = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    hits = []
    for line in out.stdout.splitlines():
        if not line:
            continue
        try:
            loc, content = line.split(':', 1)
        except ValueError:
            continue
        ln = None
        # loc may be path:line
        m = re.match(r'^(.*):(\d+)$', line)
        if m:
            fpath, ln, content = m.group(1), int(m.group(2)), m.group(3)
        else:
            continue
        rel = str(Path(fpath).relative_to(PKG_ROOT))
        c = content.strip()
        kind = 'invoke'
        km = re.match(r'invoke-(\w+)', c)
        if km:
            kind = 'invoke-' + km.group(1)
        # skip the declaration line itself (never matches invoke anyway)
        hits.append({'file': rel, 'line': ln, 'text': c[:160], 'kind': kind})
    return hits


def main():
    funcs = load_functions()
    results = []
    stats = {'java_decl': 0, 'smali_decl': 0, 'no_decl': 0}
    for idx, fn in enumerate(funcs, 1):
        jc, jm = fn['java_class'], fn['java_method']
        desc = descriptor_of(fn['params'], fn['return_type'], jc)
        # --- declaration ---
        jpath, jdecls = find_java_native_decls(jc, jm)
        decl = None
        decl_source = None
        if jdecls:
            # prefer a declaration whose signature plausibly matches; with
            # unique identity keys any native decl of this name in the class
            # file is the one (overloads have distinct params but same name;
            # disambiguate by reconstructing simple type tokens).
            want = [p.strip() for p in fn['params'].split(',')] if fn['params'].strip() else []
            best = None
            for rel, ln, txt in jdecls:
                sig = extract_java_params(txt)
                if sig == want:
                    best = (rel, ln, txt)
                    break
            if best is None and len(jdecls) == 1:
                best = jdecls[0]
            if best is not None:
                decl = {'file': 'src/java/sources/' + best[0], 'line': best[1],
                        'text': best[2]}
                decl_source = 'java'
        if decl is None:
            sd = find_smali_native_decl(jc, jm, desc)
            if sd:
                decl = sd
                decl_source = 'smali_fallback'
        if decl is None:
            results.append({'order': idx, 'jni_name': fn['jni_name'],
                            'java_class': jc, 'java_method': jm,
                            'params': fn['params'], 'return_type': fn['return_type'],
                            'descriptor': desc,
                            'reason': 'FILTERED_NON_JAVA_NATIVE',
                            'detail_decl': '', 'decl': None, 'callsite': None,
                            'java_decl_soft': None, 'smali_decl_soft':
                                find_smali_native_decl_soft(jc, jm)})
            stats['no_decl'] += 1
            continue
        stats['java_decl' if decl_source == 'java' else 'smali_decl'] += 1
        # --- callsite ---
        cs = find_smali_callsites(jc, jm, desc)
        callsite = None
        if cs:
            callsite = {'source': 'smali', **cs[0], 'n_hits': len(cs)}
        else:
            jcs = find_java_callsites(jc, jm, desc,
                                      len([p for p in fn['params'].split(',') if p.strip()]),
                                      jpath)
            if jcs:
                # prefer own-file, exact-arg-count hits
                jcs.sort(key=lambda h: (not h['own'],
                                        h['args'] is None or h['args'] != len(
                                            [p for p in fn['params'].split(',') if p.strip()])))
                best = jcs[0]
                callsite = {'source': 'java', 'file': best['file'],
                            'line': best['line'], 'text': best['text'],
                            'kind': 'java_call', 'n_hits': len(jcs)}
        results.append({'order': idx, 'jni_name': fn['jni_name'],
                        'java_class': jc, 'java_method': jm,
                        'params': fn['params'], 'return_type': fn['return_type'],
                        'descriptor': desc,
                        'reason': None if callsite else 'FILTERED_NO_JAVA_CALLSITE',
                        'decl': decl, 'decl_source': decl_source,
                        'callsite': callsite,
                        'java_decl_soft': None,
                        'smali_decl_soft': None})
    with open('/Users/syc/Person/Work/Omen/.agents/tmp/gates/evidence.json', 'w') as f:
        json.dump({'stats': stats, 'results': results}, f, indent=1)
    print(json.dumps(stats))
    from collections import Counter
    print(Counter(r['reason'] for r in results))


def extract_java_params(decl_line: str):
    """Extract parameter simple type names from a Java declaration line."""
    m = re.search(re.escape(decl_line.split('(')[0].rstrip().rsplit(' ', 1)[-1]) + r'\s*\((.*)\)', decl_line)
    if not m:
        return None
    inner = m.group(1)
    if not inner.strip():
        return []
    parts = []
    depth = 0
    cur = ''
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
        p = p.strip()
        if not p:
            continue
        toks = p.split()
        # last identifier is param name; type is the rest
        if len(toks) >= 2:
            t = ' '.join(toks[:-1])
        else:
            t = p
        # strip parameter name leaving type
        t = re.sub(r'\b(final)\b', '', t).strip()
        types.append(t)
    return types


if __name__ == '__main__':
    main()
