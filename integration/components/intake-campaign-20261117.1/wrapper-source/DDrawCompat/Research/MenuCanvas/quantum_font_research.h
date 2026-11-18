/* Private research layouts for NA1.04. Not original developer declarations.
   Packed pointer fields are 32-bit VAs/offsets, including on a 64-bit host. */
#ifndef QUANTUM_FONT_RESEARCH_H
#define QUANTUM_FONT_RESEARCH_H
#include <stdint.h>
#include <stddef.h>
#pragma pack(push,1)
typedef struct QDiskGlyphResearch {
    int16_t origin_x, origin_y;
    uint16_t width, height;
    uint32_t pixel_offset; /* relative to decoded resource start */
} QDiskGlyphResearch;
typedef struct QGlyphResearch {
    int16_t origin_x, origin_y;
    uint16_t width, height;
    uint32_t pixels; /* relocated 32-bit VA */
} QGlyphResearch;
typedef struct QFontHeaderResearch {
    uint8_t first_code, last_code, pixel_bits;
    int8_t tracking;
    uint8_t unknown_04, max_y_extent;
    int8_t baseline_adjust;
    uint8_t unknown_07;
    uint16_t unknown_08, glyph_count;
    /* glyph_count QGlyphResearch records begin at +0x0C. */
} QFontHeaderResearch;
typedef struct QImageResearch {
    uint16_t flags;
    uint8_t unknown_02, and_mask, xor_value, pixel_bits;
    int16_t origin_x, origin_y;
    uint16_t width, height;
    uint32_t surface; /* +0x0E: 32-bit COM interface VA when surface flag set */
    uint32_t pixels;  /* +0x12: 32-bit CPU byte pointer */
    int16_t pitch;   /* +0x16: cached pitch; truncation/large-pitch limits untested */
} QImageResearch;
typedef struct QFontStateResearch {
    uint32_t font; /* QFontHeaderResearch VA */
    uint8_t blit_flags, image_tag, and_mask, xor_value;
    uint8_t effects, alignment;
    uint16_t background_gate;
    int16_t pen_x, pen_y;
    uint16_t layout_width, layout_height;
    int16_t effect_dx, effect_dy;
    uint8_t mono_index, unknown_19;
    uint16_t background_value;
    uint8_t effect_index, unknown_1d;
} QFontStateResearch;
#pragma pack(pop)
#if defined(__cplusplus)
static_assert(sizeof(QDiskGlyphResearch)==12,"disk glyph stride");
static_assert(sizeof(QGlyphResearch)==12,"runtime glyph stride");
static_assert(sizeof(QFontHeaderResearch)==12,"runtime header");
static_assert(sizeof(QImageResearch)==24,"packed image descriptor");
static_assert(sizeof(QFontStateResearch)==30,"font state source record");
static_assert(offsetof(QImageResearch,surface)==0x0E,"unaligned surface pointer");
static_assert(offsetof(QImageResearch,pixels)==0x12,"unaligned CPU pointer");
static_assert(offsetof(QFontStateResearch,pen_x)==0x0C,"font pen X");
static_assert(offsetof(QFontStateResearch,effect_index)==0x1C,"effect index");
#endif
#endif
