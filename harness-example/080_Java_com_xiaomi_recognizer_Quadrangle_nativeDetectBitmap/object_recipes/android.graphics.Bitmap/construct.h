/* Omen Algorithm 2 recipe interface — android.graphics.Bitmap (ARGB_8888).
 * See construct.c and recipe.md for the selected path and evidence. */
#ifndef OMEN_CONSTRUCT_ANDROID_GRAPHICS_BITMAP_H
#define OMEN_CONSTRUCT_ANDROID_GRAPHICS_BITMAP_H

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Create a REAL android.graphics.Bitmap (ARGB_8888) of width x height whose
 * pixels are the RGBA bytes in `pixels` (>= width*height*4 bytes).
 *
 * width/height are CONSTRUCTION-AXIS parameters (caller's deterministic
 * pattern table) — never fuzz-decoded; the pixel payload is the fuzzed or
 * pattern-generated content. Zero fuzz bytes are consumed inside.
 *
 * Returns a new local reference (caller deletes), or NULL on failure with a
 * "[RECIPE] CONSTRUCT_FAIL(android.graphics.Bitmap)" breadcrumb on stderr
 * and any pending Java exception cleared. */
jobject construct_android_graphics_Bitmap(JNIEnv* env,
                                          jint width, jint height,
                                          const uint8_t* pixels,
                                          size_t pixel_len);

#ifdef __cplusplus
}
#endif

#endif /* OMEN_CONSTRUCT_ANDROID_GRAPHICS_BITMAP_H */
