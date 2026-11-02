"""Java-type <-> smali-descriptor conversion helpers for the target gate."""
import re

# Primitive / builtin type name -> descriptor token
PRIM = {
    "void": "V", "boolean": "Z", "byte": "B", "short": "S",
    "char": "C", "int": "I", "long": "J", "float": "F", "double": "D",
}

_ARRAY_SPLIT = re.compile(r"\[\]")


def type_to_descriptor(t: str) -> str:
    """Convert a Java type name like 'byte[][]' or 'java.lang.String'
    (or a raw simple name like 'NativeRef.EVP_PKEY') into a smali descriptor."""
    t = t.strip()
    if not t:
        raise ValueError("empty type")
    dims = 0
    while t.endswith("[]"):
        dims += 1
        t = t[:-2].strip()
    if t in PRIM:
        base = PRIM[t]
    elif "." in t and not t[0].isupper() and "/" not in t:
        # Looks like a fully-qualified class (e.g. com.example.Foo) only when it
        # contains a lowercase-first segment. Heuristic is avoided: caller passes
        # resolution context. Fallback: treat dot-name as class name.
        base = "L" + t.replace(".", "/") + ";"
    else:
        # Simple or nested name; cannot resolve package here -> placeholder.
        base = "L__UNRESOLVED__" + t.replace(".", "/") + ";"
    return "[" * dims + base


def split_params(params: str):
    """Split a comma-separated param string into type names. Empty -> []."""
    params = params.strip()
    if not params:
        return []
    return [p.strip() for p in params.split(",")]


def descriptor_of(params: str, ret: str, resolver=None) -> str:
    """Build a smali method descriptor '(...)R'.

    `resolver` maps a simple/nested type name from jni_list to a fully
    qualified class name (FQCN) using knowledge of the app's class universe.
    """
    def conv(t: str) -> str:
        t = t.strip()
        dims = 0
        while t.endswith("[]"):
            dims += 1
            t = t[:-2].strip()
        if t in PRIM:
            base = PRIM[t]
        else:
            fq = resolver(t) if resolver else None
            if fq is None:
                # A type that already looks fully qualified (has a lowercase
                # leading package segment) is used as-is.
                if re.match(r"^[a-z][A-Za-z0-9_]*(\.[A-Za-z0-9_$]+)+$", t):
                    fq = t
                else:
                    raise ValueError("unresolvable:" + t)
            base = "L" + fq.replace(".", "/") + ";"
        return "[" * dims + base

    return "(" + "".join(conv(p) for p in split_params(params)) + ")" + conv(ret)
