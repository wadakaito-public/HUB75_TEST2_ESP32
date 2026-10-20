#ifndef FONT_DRAW_H//多重includeガード
#define FONT_DRAW_H

#include<stdint.h>
#include<stddef.h>
#include<stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Font_8px_Misaki,
    Font_12px_Denkichip
} FontKind;

typedef struct {
    uint16_t code;
    size_t width; // 字幅
    size_t height; //字高
    size_t top_offset; //行上から字形上端までのオフセット
} PropWidth;

typedef struct {
    FontKind kind;
    size_t width;
    size_t width_halffont;
    size_t width_kanafont;
    size_t offset_normalfont; //左にxドットずらす　ex)x = 2, 00110011 → 11001100
    size_t offset_halffont;
    size_t offset_kanafont;
    size_t height;
    bool proportional; //可変幅フォントか否か。今回はA-Z,a-zのみとする。
    const PropWidth *prop_width;
    size_t prop_width_count;
} FontInfo;

const FontInfo *font_get_info(FontKind kind);

typedef struct {
    size_t width;
    size_t height;
    const void *data;
    bool (*pixelset) (const void *data, size_t x, size_t y, size_t offset); 
} FontGlyph;

typedef struct{
    size_t width;
    size_t height;
    size_t stride; //一行のバイト数 (width + 7) /8→+7で端数切り上げ
    uint8_t **data; //ドットデータ
} StrBitmap;

bool font_draw(FontKind font, const char *string, StrBitmap *out);
void font_drawdata_free(StrBitmap *bitmap);

#ifdef __cplusplus
}
#endif

#endif//FONT_DRAW_H