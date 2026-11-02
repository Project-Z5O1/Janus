"""Shared loaders/cache for the com.heytap.market target gate."""
import json
import os
import re
import sys
from pathlib import Path

PKG_ROOT = Path("/Users/syc/Person/Work/Omen/target_APK/com.heytap.market")
JAVA_ROOT = PKG_ROOT / "src" / "java" / "sources"
SMALI_ROOTS = sorted((PKG_ROOT / "src" / "smali").glob("smali*"))
NATIVE_ROOT = PKG_ROOT / "src" / "native" / "arm64-v8a"

PRIM = {
    "void": "V", "boolean": "Z", "byte": "B", "short": "S",
    "char": "C", "int": "I", "long": "J", "float": "F", "double": "D",
}

# FQCN resolution for simple/nested type names appearing in jni_list params.
# Verified per owning class against JADX imports (see analysis notes).
FQCN = {
    "AbstractSessionContext": "org.conscrypt.AbstractSessionContext",
    "AtomicReference": "java.util.concurrent.atomic.AtomicReference",
    "Bitmap": "android.graphics.Bitmap",
    "BitmapFactory.Options": "android.graphics.BitmapFactory$Options",
    "Buffer": "java.nio.Buffer",
    "ByteBuffer": "java.nio.ByteBuffer",
    "Calendar": "java.util.Calendar",
    "Class": "java.lang.Class",
    "ColorInfo": "com.oplus.tbl.exoplayer2.video.ColorInfo",
    "Context": "android.content.Context",
    "FileDescriptor": "java.io.FileDescriptor",
    "FrameOutputBuffer": "com.oplus.tblplayer.ffmpeg.FrameOutputBuffer",
    "HashMap": "java.util.HashMap",
    "InputStream": "java.io.InputStream",
    "Object": "java.lang.Object",
    "OutputStream": "java.io.OutputStream",
    "PrivateKey": "java.security.PrivateKey",
    "QFile": "tmsdk.common.tcc.QFile",
    "SSLHandshakeCallbacks": "org.conscrypt.NativeCrypto.SSLHandshakeCallbacks",
    "String": "java.lang.String",
    "Surface": "android.view.Surface",
    "WebPFrame": "com.facebook.animated.webp.WebPFrame",
    "WebPImage": "com.facebook.animated.webp.WebPImage",
    "ZstdDictCompress": "com.heytap.zstd.ZstdDictCompress",
    "ZstdDictDecompress": "com.heytap.zstd.ZstdDictDecompress",
}

FQCN_OCONSCRYPT = dict(FQCN)
FQCN_OCONSCRYPT.update({
    "AbstractSessionContext": "org.oconscrypt.AbstractSessionContext",
    "NativeRef.CMAC_CTX": "org.oconscrypt.NativeRef$CMAC_CTX",
    "NativeRef.EC_GROUP": "org.oconscrypt.NativeRef$EC_GROUP",
    "NativeRef.EC_POINT": "org.oconscrypt.NativeRef$EC_POINT",
    "NativeRef.EVP_CIPHER_CTX": "org.oconscrypt.NativeRef$EVP_CIPHER_CTX",
    "NativeRef.EVP_MD_CTX": "org.oconscrypt.NativeRef$EVP_MD_CTX",
    "NativeRef.EVP_PKEY": "org.oconscrypt.NativeRef$EVP_PKEY",
    "NativeRef.EVP_PKEY_CTX": "org.oconscrypt.NativeRef$EVP_PKEY_CTX",
    "NativeRef.HMAC_CTX": "org.oconscrypt.NativeRef$HMAC_CTX",
    "NativeSsl": "org.oconscrypt.NativeSsl",
    "OpenSSLBIOInputStream": "org.oconscrypt.OpenSSLBIOInputStream",
    "OpenSSLX509CRL": "org.oconscrypt.OpenSSLX509CRL",
    "OpenSSLX509Certificate": "org.oconscrypt.OpenSSLX509Certificate",
    "SSLHandshakeCallbacks": "org.oconscrypt.NativeCrypto$SSLHandshakeCallbacks",
})

FQCN_CONSCRYPT = dict(FQCN)
FQCN_CONSCRYPT.update({
    "NativeRef.CMAC_CTX": "org.conscrypt.NativeRef$CMAC_CTX",
    "NativeRef.EC_GROUP": "org.conscrypt.NativeRef$EC_GROUP",
    "NativeRef.EC_POINT": "org.conscrypt.NativeRef$EC_POINT",
    "NativeRef.EVP_CIPHER_CTX": "org.conscrypt.NativeRef$EVP_CIPHER_CTX",
    "NativeRef.EVP_MD_CTX": "org.conscrypt.NativeRef$EVP_MD_CTX",
    "NativeRef.EVP_PKEY": "org.conscrypt.NativeRef$EVP_PKEY",
    "NativeRef.EVP_PKEY_CTX": "org.conscrypt.NativeRef$EVP_PKEY_CTX",
    "NativeRef.HMAC_CTX": "org.conscrypt.NativeRef$HMAC_CTX",
    "NativeSsl": "org.conscrypt.NativeSsl",
    "OpenSSLBIOInputStream": "org.conscrypt.OpenSSLBIOInputStream",
    "OpenSSLX509CRL": "org.conscrypt.OpenSSLX509CRL",
    "OpenSSLX509Certificate": "org.conscrypt.OpenSSLX509Certificate",
    "SSLHandshakeCallbacks": "org.conscrypt.NativeCrypto$SSLHandshakeCallbacks",
})


def resolver_for(java_class: str):
    if java_class.startswith("org.oconscrypt."):
        return FQCN_OCONSCRYPT
    if java_class.startswith("org.conscrypt."):
        return FQCN_CONSCRYPT
    return FQCN


def descriptor_of(params: str, ret: str, java_class: str) -> str:
    table = resolver_for(java_class)

    def conv(t: str) -> str:
        t = t.strip()
        dims = 0
        while t.endswith("[]"):
            dims += 1
            t = t[:-2].strip()
        if t in PRIM:
            base = PRIM[t]
        else:
            fq = table.get(t)
            if fq is None:
                if re.match(r"^[a-z][A-Za-z0-9_]*(\.[A-Za-z0-9_$]+)+$", t):
                    fq = t
                else:
                    raise ValueError("unresolvable type: " + t)
            # Nested classes use '$' in smali descriptors; the raw signature
            # file preserved '.', so normalize when the FQCN tail is a known
            # nested-class owner (NativeRef.* / SSLHandshakeCallbacks).
            base = "L" + fq.replace(".", "/") + ";"
        return "[" * dims + base

    ps = [p.strip() for p in params.split(",")] if params.strip() else []
    return "(" + "".join(conv(p) for p in ps) + ")" + conv(ret)


def load_functions():
    with open(PKG_ROOT / "jni_list.json") as f:
        return json.load(f)["functions"]


def smali_path_for(java_class: str):
    rel = java_class.replace(".", "/") + ".smali"
    for root in SMALI_ROOTS:
        p = root / rel
        if p.exists():
            return p
    return None
