//
//  jif.c
//  Mangamix
//
//  Created by theme on 19/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jif.h"


jif_offset jif_get_offset(JIF_SCANNER * s){
    return s->b / 8;
}

bool jif_is_marker_byte(byte b) {
    return (0x00 < b) && (b < 0xFF); /* Note all byte in open range (0x00, 0xff) are possible markers */
}

/* if (return < to), found a marker */
JIF_SCANNER * jif_new_scanner(byte * jif_array, jif_offset array_size){
    JIF_SCANNER * s = (JIF_SCANNER *)malloc(sizeof(JIF_SCANNER));
    s->pjif = jif_array;
    s->size = array_size * 8;
    s->m = 0;
    s->b = 0;
    return s;
}

JIF_SCANNER * jif_copy_scanner(JIF_SCANNER * s){
    JIF_SCANNER * news = (JIF_SCANNER *)malloc(sizeof(JIF_SCANNER));
    memcpy(news, s, sizeof(JIF_SCANNER));
    return news;
}

void jif_del_scanner(JIF_SCANNER * s){
    free(s);
}

bool jif_scan_next_marker(JIF_SCANNER * s){
    byte* p = s->pjif;    /* helper for shorter code */
    
    for(; s->b < (s->size - 8); s->b += 8 ) {
        if( (p[s->b / 8] == 0xFF) && jif_is_marker_byte(p[(s->b / 8 )+1]) ) {   /* find a marker */
            s->b += 8;  /* point to marker content */
            s->m = s->b / 8;
            return true;
        }
    }
    return false;
}

JIF_MARKER jif_prob_next_marker(JIF_SCANNER * s){
    byte* p = s->pjif;    /* helper for shorter code */
    jif_offset i = s->b;
    for(; i < (s->size -8); i += 8) {
        if( (p[i/8] == 0xFF) && jif_is_marker_byte(p[(i/8)+1]) ) {   /* find a marker */
            return p[(i/8)+1];
        }
    }
    return 0x00;
}

JIF_MARKER jif_current_marker(JIF_SCANNER * s){
    return s->pjif[s->m];
}

bool jif_scan_next_maker_of(JIF_MARKER m, JIF_SCANNER * s ){
    
    while ( jif_scan_next_marker(s) ){
        if (m == jif_current_marker(s)){
            return true;
        }
    }
    return false;
}


byte jif_current_byte(JIF_SCANNER * s){
    return s->pjif[s->b / 8];
}

byte jif_next_byte(JIF_SCANNER * s){
    return s->pjif[(s->b +8) / 8];
}

byte jif_scan_next_byte(JIF_SCANNER * s){
    s->b += 8;
    return s->pjif[s->b / 8];
}

uint16_t jif_scan_2_bytes(JIF_SCANNER * s){
    return (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
}

uint32_t jif_scan_4_bytes(JIF_SCANNER * s){
    return  (jif_scan_next_byte(s) << 24) + ( jif_scan_next_byte(s) << 16 )
    + ( jif_scan_next_byte(s) << 8 ) + ( jif_scan_next_byte(s) );
}

uint8_t jif_bit_in_byte(JIF_SCANNER * s){
    return s->b % 8;
}

uint16_t jif_scan_t_bits(JIF_SCANNER * s, jif_offset t){
    uint16_t bits;
    jif_offset byte1 = s->b / 8, byte2 = (s->b + t) / 8;
    if ( byte1 == byte2 ){  // in the same byte
        /* read this byte */
        bits = s->pjif[byte1];
        /* right shift */
        bits = bits >> ( 8 - (s->b%8) - t);
    } else {    // across two bytes
        /* read two bytes */
        bits = s->pjif[byte1];
        bits = bits << 8;
        bits += s->pjif[byte2];
        
        /* right shift */
        bits = bits >> ( 16 - (s->b%8) - t);
    }
    
    s->b += t;
    return bits;
}

uint16_t jif_scan_next_bit(JIF_SCANNER * s){
    return jif_scan_t_bits(s, 1);
}


