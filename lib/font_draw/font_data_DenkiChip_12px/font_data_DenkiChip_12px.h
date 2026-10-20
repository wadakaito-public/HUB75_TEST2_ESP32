#ifndef FONT_DATA_DENKICHIP_12PX_H
#define FONT_DATA_DENKICHIP_12PX_H

#include <stdint.h>
#include <stdbool.h>
#include "../font_draw.h"

#ifdef __cplusplus
extern "C" {
#endif

bool get_glyph_12px_DenkiChip(uint16_t codepoint, FontGlyph *out);
extern const FontInfo Font_12px_Denkichip_info;

#ifdef __cplusplus
}
#endif

#endif