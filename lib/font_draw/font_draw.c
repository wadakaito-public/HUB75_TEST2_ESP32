//とりあえず完成

#include <stdio.h>
#include <stdint.h>//固定長int型ライブラリ
#include <stddef.h>//型のサイズ取得
#include <stdbool.h>
#include <stdlib.h>
#include "font_draw.h"
#include "font_data_8px/font_data_8px.h"
#include "font_data_DenkiChip_12px/font_data_DenkiChip_12px.h"

static const FontInfo *const font_info_table[] = {
    &Font_8px_Misaki_info,
    &Font_12px_Denkichip_info,
};
const FontInfo *font_get_info(FontKind kind)
{
    size_t count = sizeof(font_info_table) / sizeof(font_info_table[0]);

    for (size_t i = 0; i < count; i++) {
        if (font_info_table[i]->kind == kind) {
            return font_info_table[i];
        }
    }

    return NULL;  // 未登録の FontKind
}

static uint16_t utf8_to_unicode(const char *utf8, size_t *consumed) {//バイト長記録変数のアドレスを受け取る
    uint8_t byte1 = utf8[0]; //1byte目記録
    /*
    UTF-8の3byte文字の構造
    1110xxxx 10xxxxxx 10xxxxxx
    xの部分がUnicodeのビット
    */
    if((byte1 & 0x80) == 0) { //1byte文字は先頭bitが0→0x80(10000000)と論理積
        if(consumed != NULL) *consumed = 1; //引数の値を3に書き換える
        uint16_t code = byte1;
        return code;
    }
    if((byte1 & 0xE0) == 0xC0) { //2byte 上3bit取り出し→&0xE0 110XXXXXか判定
        uint8_t byte2 = utf8[1];
        uint16_t code = 
            ((byte1 & 0x1f) << 6) //110xxxxx 10xxxxxx UTF=8の2byte文字
            |(byte2 & 0x3f);
        if (consumed != NULL) *consumed = 2;
        return code;
    }
    
    if((byte1 & 0xF0) == 0xE0) { //3byte文字は先頭4bitが1110→0xF0との論理積をとって、先頭4bitを取り出し判定
        uint8_t byte2 = utf8[1];
        uint8_t byte3 = utf8[2];
        uint16_t code =
            ((byte1 & 0x0f) << 12) //先頭8bit中、下位4bitがUnicodeのbitなので、0x(hex)0f→00001111と論理積、下位12bitのためビットシフト
            |((byte2 & 0x3f) << 6) //0x3f 00111111
            |((byte3 & 0x3f));
        if(consumed != NULL) *consumed = 3;
        return code;
    }

    if ((byte1 & 0xF8) == 0xF0) {
        uint8_t byte2 = utf8[1];
        uint8_t byte3 = utf8[2];
        uint8_t byte4 = utf8[3];
        uint32_t code = ((uint32_t)(byte1 & 0x07) << 18) |
                       ((uint32_t)(byte2 & 0x3F) << 12) |
                       ((uint32_t)(byte3 & 0x3F) << 6) |
                       (uint32_t)(byte4 & 0x3F);
        if (consumed) *consumed = 4;
        return (uint16_t)code; // ここでは16bit前提
    }

    if (consumed) *consumed = 1;
    return 0;
}
void font_drawdata_free(StrBitmap *bitmap){
    if(bitmap == NULL || bitmap->data == NULL)return;
    for(size_t row = 0; row < bitmap->height; row++){
        free(bitmap->data[row]);
    }
    free(bitmap->data);
    bitmap->data = NULL;
}

bool get_font_glyph(uint16_t codepoint, FontKind font, FontGlyph *out){
    switch (font)
    {
    case Font_8px_Misaki:
        if(!get_glyph_8px_misaki(codepoint, out))return false;
        break;
    case Font_12px_Denkichip:
        if(!get_glyph_12px_DenkiChip(codepoint, out))return false;
        break;
    default:
        return false;
    }
    return true;
}

bool get_string_dotdata(const char *str, FontKind font, StrBitmap *out){
    if(str == NULL || out == NULL)return false;
    StrBitmap bitmap;
    const FontInfo *info = font_get_info(font);
    if(info == NULL) return false;
    bitmap.height = info->height;
    bitmap.width = 0;
    bitmap.stride = 0;
    size_t char_count = 0;
    const char *head = str;

    if(*str == '\0') {
        bitmap.data = NULL;
        *out =bitmap;
        return true;
    }
    /*====一回目の走査開始、幅を確定=================*/
    while (*str != '\0') { //ポインタ終わりまでループ
        size_t char_size = 0;
        uint16_t code = utf8_to_unicode(str, &char_size);
        if(char_size == 0)break;//何らかの不正
        if(char_count >= 100)break;
        
        if((0x0020 <= code && code <= 0x007E)//ASCII文字(半角英数記号)
        ||(0xff61 <= code && code <= 0xff9f))//半角カタカナ
        {
            if(info->proportional){
                size_t width = info->width_halffont;
                for(size_t index = 0; index < info->prop_width_count; index++)
                {
                    if(info->prop_width[index].code == code){
                        width = info->prop_width[index].width;
                        break;
                    }
                    
                }
                bitmap.width += width;
            }else{
                bitmap.width += info->width_halffont;
            }
        } else if( (0x3040 <= code && code <= 0x309F) //ひらがな
        ||(0x30A0 <= code && code <= 0x30FF)) //カタカナ
        {
            bitmap.width += info->width_kanafont;
        }else {
            bitmap.width += info->width; //それ以外は全角
        }
        char_count++;
        str += char_size;//文字のサイズ分ポインタを進める
    }
    /*=========一回目の走査終了======================*/
    
    //バイト配列確保
     bitmap.stride = (bitmap.width + 7) / 8;//端数切り上げでバイト長にする
    bitmap.data = calloc(bitmap.height, sizeof *bitmap.data); //ポインタをbitmap.height個用意、0で初期化
    if(bitmap.data == NULL){
        free(bitmap.data);
        return false;
    }
    for (size_t y = 0; y < bitmap.height; y++){
        bitmap.data[y] = calloc(bitmap.stride, sizeof *bitmap.data[y]);//１byteのuint_t* をstride個(バイト数)、0埋め
        if(bitmap.data[y] == NULL) {
            for(size_t row = 0; row < bitmap.height; row++){
                free(bitmap.data[row]);
            }
            free(bitmap.data);
            return false;
        }
    }

    /*===========ドットデータを実際に入れる================================================================*/
    int cursor = 0; //文字を入れるビット列の左端
    const char *current = head;
    for(size_t i = 0; i < char_count; i++) {
        //一文字ずつ左端から詰めていく
        size_t char_size;//文字サイズ受け取り変数
        size_t char_width;
        size_t offset;
        size_t top_offset = 0;
        size_t row_shift = 0;
        size_t char_top = info->height;
        size_t first_on_column = info->width; // 12列すべて空白の場合の値
        uint16_t code =utf8_to_unicode(current, &char_size);
        FontGlyph char_bitmap;//文字データ用領域
        if(!get_font_glyph(code,font,&char_bitmap)){//文字のドットデータ取得
            font_drawdata_free(&bitmap);
            return false;
        }
        if((0x0020 <= code && code <= 0x007E)//ASCII文字(半角英数記号)
        ||(0xff61 <= code && code <= 0xff9f))//半角カタカナ
        {
            if(info->proportional){
                bool catch = false;
                for(size_t index = 0; index < info->prop_width_count; index++)
                {
                    //可変幅取得
                    if(info->prop_width[index].code == code){ 
                        char_width = info->prop_width[index].width;
                        top_offset = info->prop_width[index].top_offset;
                        catch = true;
                        break;
                    }
                    
                }
                if(!catch){ //該当なし
                        char_width = info->width_halffont; //該当がないので取り敢えずデフォルト値をセット
                        offset = info->offset_halffont;
                    }
                //文字の左端,上端取得
                for (size_t y = 0; y < char_bitmap.height; y++) {
                    for (size_t x = 0; x < char_bitmap.width; x++) {
                        if (char_bitmap.pixelset(char_bitmap.data, x, y, 0)) {
                            if (x < first_on_column) {
                                first_on_column = x;
                            }
                            if(y < char_top){
                                char_top = y;
                            }
                        }
                    }
                }
                offset = first_on_column;
                if (char_top < char_bitmap.height) {
                    row_shift = top_offset - char_top;
                }
                printf("char_top:%zu,top_offset:%zu  ",char_top,top_offset);
                
            }else{
                char_width = info->width_halffont;
                offset = info->offset_halffont;
            }
        }else if( (0x3040 <= code && code <= 0x309F) //ひらがな
        ||(0x30A0 <= code && code <= 0x30FF)) //カタカナ
        {
            char_width = info->width_kanafont;
            offset = info->offset_kanafont;
        }else {
            char_width = info->width; //それ以外は全角
            offset = info->offset_normalfont;
        }
        
        for(size_t y = 0; y < char_bitmap.height; y++){
            size_t y_plot = y + row_shift;
            if (y_plot >= bitmap.height) continue;
            for(size_t x = 0; x < char_width; x++){
                int bit_position = (cursor + x)%8;
                int byte_index = (cursor + x)/8;
                int bit_mask = 0x80 >> bit_position;
                if(char_bitmap.pixelset(char_bitmap.data,x,y,offset))bitmap.data[y_plot][byte_index] |= bit_mask;
            }
        }
        cursor += char_width;
        current += char_size;//ポインタを文字サイズ分ずらす
    }

    *out = bitmap;
    return true;
}

void draw_String_System_out(const char *string,FontKind font) {
    StrBitmap bitmap;
    if (!get_string_dotdata(string, font, &bitmap)) {
        printf("font not found or input is not one character\n");
        return;
    }
    printf("\n");
    for (size_t row = 0; row < bitmap.height; row++) { //行セレクタ
        for(size_t byte_index = 0; byte_index < bitmap.stride; byte_index++){ //column セレクタ、左から何バイト目のバイト列か
            uint8_t byte = bitmap.data[row][byte_index];
            for(size_t bit_index = 0; bit_index < 8; bit_index++) { // bitセレクタ、選択したバイト列を実際に読み取る
                if(byte & (0x80 >> bit_index)){ //byte: 0b00110011 と、0b10000000 で積をとる→0b10000000を右にビットシフト、隣のビットと比較する
                printf("##");
                }else{
                printf("__");
                }
            }
        }
        printf("\n");
    }
}

int main (void){
    //draw_String_System_out("あ1A機?安",Font_8px_Misaki);
    /*draw_String_System_out("あいうえお",Font_12px_Denkichip);
    draw_String_System_out("1",Font_12px_Denkichip);
    draw_String_System_out("w",Font_12px_Denkichip);
    draw_String_System_out("y",Font_12px_Denkichip);
    draw_String_System_out("i",Font_12px_Denkichip);
    draw_String_System_out("a",Font_12px_Denkichip);
    draw_String_System_out("<",Font_12px_Denkichip);
    draw_String_System_out("&",Font_12px_Denkichip);
    printf("\n");
    draw_String_System_out("円円1円",Font_12px_Denkichip);
    draw_String_System_out("1234567890",Font_12px_Denkichip);
    draw_String_System_out("0",Font_12px_Denkichip);
    draw_String_System_out("Hallo_",Font_12px_Denkichip);
    draw_String_System_out("World!!.`",Font_12px_Denkichip);*/
    draw_String_System_out("jklmopq_",Font_12px_Denkichip);
    return 0;
}//__##############