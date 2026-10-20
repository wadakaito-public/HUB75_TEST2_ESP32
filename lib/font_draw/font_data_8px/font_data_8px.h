#ifndef FONT_DATA_8PX_H
#define FONT_DATA_8PX_H

#include <stdint.h>
#include <stdbool.h>
#include "../font_draw.h"

#ifdef __cplusplus
extern "C" {
#endif

bool get_glyph_8px_misaki(uint16_t codepoint, FontGlyph *out);
extern const FontInfo Font_8px_Misaki_info;

#ifdef __cplusplus
}
#endif

#endif