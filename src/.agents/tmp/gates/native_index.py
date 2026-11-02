"""Native pseudocode index: locate function body by library + offset."""
import re
from pathlib import Path

NATIVE_ROOT = Path('/Users/syc/Person/Work/Omen/target_APK/com.heytap.market/src/native/arm64-v8a')

HEADER_RE = re.compile(
    r'^/\*\s*(?P<name>\S+)\s+@\s+(?P<entry>[0-9a-fA-F]+)\s+'
    r'\(offset\s+(?P<off>0x[0-9a-fA-F]+),\s*imagebase\s+(?P<base>[0-9a-fA-F]+)\)\s*\*/\s*$')

_index = None


def build_index():
    global _index
    if _index is not None:
        return _index
    idx = {}  # (lib_stem, normalized_offset_int) -> {file,line,name,body_range}
    for p in sorted(NATIVE_ROOT.glob('*.c')):
        lib = p.stem
        lines = p.read_text(errors='replace').splitlines()
        headers = []  # (line_idx0, offset_int, name)
        for i, line in enumerate(lines):
            m = HEADER_RE.match(line.strip())
            if m:
                off = int(m.group('off'), 16)
                headers.append((i, off, m.group('name')))
        for j, (i, off, name) in enumerate(headers):
            end = headers[j + 1][0] if j + 1 < len(headers) else len(lines)
            idx.setdefault((lib, off), []).append({
                'file': 'src/native/arm64-v8a/' + p.name,
                'line': i + 1, 'name': name,
                'body': lines[i:end],
                'start': i, 'end': end})
    _index = idx
    return idx


def find_function(library: str, offset_hex: str):
    """library like 'libfoo.so'; offset like '0x1234'."""
    lib = library[:-3] if library.endswith('.so') else library
    off = int(offset_hex, 16)
    idx = build_index()
    return idx.get((lib, off))
