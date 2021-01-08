
// #include "globals.h"

// char buffer[20];
// U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// void oledInit() {
//     u8x8.begin();
//     u8x8.setFont(u8x8_font_chroma48medium8_r); 
//     u8x8.setContrast(128);
//     // u8x8.clear();
// }

// void oledVerHora(st_hora h) {
//     // sprintf(buffer, "%02d:%02d:%02d", h.hora, h.min, h.seg);
//     sprintf(buffer, "%02d:%02d", h.hora, h.min);
//     u8x8.setFont(u8x8_font_7x14B_1x2_f);
//     u8x8.draw2x2String(3,2, buffer);
// }

// void oledVerTemp(float t, float h) {
//     u8x8.setFont(u8x8_font_chroma48medium8_r);
//     if( !isnan(t) ) {
//         sprintf(buffer, "T:%2.1fC H:%2.1f", t, h);
//     } else {
//         sprintf(buffer,"error temp ");
//     }
//     u8x8.drawString(1,16, buffer);
// }

