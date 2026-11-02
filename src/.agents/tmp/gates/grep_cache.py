"""Shared caches: java file index + smali invoke index.

The smali invoke index is built ONCE by scanning all .smali lines with a
single regex, mapping 'Lowner;->name(desc' -> [(file, line, kind, text)].
Java native-decl index maps java_class -> [(file, line, text)] from the
class's own file.
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, '/Users/syc/Person/Work/Omen/.agents/tmp/gates')
from gate_common import PKG_ROOT, JAVA_ROOT, SMALI_ROOTS

SMALI_INVOKE_RE = re.compile(
    r'^\s*(invoke-(?:virtual|static|direct|super|interface|range|polymetric|custom|custom/range)'
    r'(?:/range)?)\s+\{[^}]*\},\s*(L[\w/$;.\[]+)->([^\s(]+)(\([^)]*\).*)$')

_smali_index = None
_java_native_index = None
_smali_method_index = None


def smali_invoke_index():
    global _smali_index
    if _smali_index is not None:
        return _smali_index
    idx = {}
    n_files = 0
    for root in SMALI_ROOTS:
        for p in root.rglob('*.smali'):
            n_files += 1
            rel = 'src/smali/' + str(p.relative_to(PKG_ROOT / 'src' / 'smali'))
            try:
                with open(p, encoding='utf-8', errors='replace') as f:
                    for i, line in enumerate(f, 1):
                        if 'invoke-' not in line:
                            continue
                        m = SMALI_INVOKE_RE.match(line)
                        if not m:
                            continue
                        kind, owner, name, desc = m.groups()
                        key = owner + '->' + name + desc
                        idx.setdefault(key, []).append(
                            (rel, i, kind.strip(), line.strip()[:200]))
            except OSError:
                continue
    _smali_index = idx
    return idx


def java_native_index():
    """java_class -> list of (relfile, line, stripped_line) for native decls."""
    global _java_native_index
    if _java_native_index is not None:
        return _java_native_index
    idx = {}
    for p in JAVA_ROOT.rglob('*.java'):
        rel = 'src/java/sources/' + str(p.relative_to(JAVA_ROOT))
        fq = str(p.relative_to(JAVA_ROOT))[:-5].replace('/', '.')
        try:
            with open(p, encoding='utf-8', errors='replace') as f:
                for i, line in enumerate(f, 1):
                    ls = line.strip()
                    if 'native' not in ls:
                        continue
                    if not re.search(r'\bnative\b', ls):
                        continue
                    if not re.search(r'\(', ls):
                        continue
                    if ls.startswith('//') or ls.startswith('*') or ls.startswith('/*'):
                        continue
                    idx.setdefault(fq, []).append((rel, i, ls))
        except OSError:
            continue
    _java_native_index = idx
    return idx


def smali_method_index():
    """java_class -> list of dicts for .method entries with native flag."""
    global _smali_method_index
    if _smali_method_index is not None:
        return _smali_method_index
    idx = {}
    meth_re = re.compile(
        r'^\.method\s+(?P<acc>.+?)\s+(?P<name>[^\s(]+)(?P<desc>\([^)]*\).*)$')
    for root in SMALI_ROOTS:
        for p in root.rglob('*.smali'):
            rel = 'src/smali/' + str(p.relative_to(PKG_ROOT / 'src' / 'smali'))
            _rel = str(p.relative_to(PKG_ROOT / 'src' / 'smali'))
            _parts = _rel.split('/')
            if _parts[0].startswith('smali'):
                _parts = _parts[1:]
            fq = '/'.join(_parts)[:-6].replace('/', '.')
            try:
                with open(p, encoding='utf-8', errors='replace') as f:
                    for i, line in enumerate(f, 1):
                        if not line.startswith('.method'):
                            continue
                        m = meth_re.match(line.strip())
                        if not m:
                            continue
                        idx.setdefault(fq, []).append({
                            'file': rel, 'line': i, 'text': line.strip(),
                            'native': 'native' in m.group('acc'),
                            'name': m.group('name'), 'desc': m.group('desc')})
            except OSError:
                continue
    _smali_method_index = idx
    return idx


def java_import_index():
    """java_class -> set of files importing it."""
    global _java_import_index
    if _java_import_index is not None:
        return _java_import_index
    idx = {}
    imp_re = re.compile(r'^import\s+(?:static\s+)?([\w.$]+)\s*;')
    for p in JAVA_ROOT.rglob('*.java'):
        rel = 'src/java/sources/' + str(p.relative_to(JAVA_ROOT))
        try:
            with open(p, encoding='utf-8', errors='replace') as f:
                for line in f:
                    if not line.startswith('import '):
                        continue
                    m = imp_re.match(line.strip())
                    if m:
                        idx.setdefault(m.group(1), set()).add(rel)
        except OSError:
            continue
    _java_import_index = idx
    return idx


_java_import_index = None
