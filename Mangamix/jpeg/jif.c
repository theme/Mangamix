//
//  jif.c
//  Mangamix
//
//  Created by theme on 19/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jif.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> /* memcpy() */

bool jif_is_marker_byte(byte b) {
    return (0x00 < b) && (b < 0xFF); /* Note all byte in open range (0x00, 0xff) are possible markers */
}

/* if (return < to), found a marker */
JIF_SCANNER * jif_new_scanner(byte * jif_array, jif_offset array_size){
    JIF_SCANNER * s = (JIF_SCANNER *)malloc(sizeof(JIF_SCANNER));
    s->pjif = jif_array;
    s->size = array_size;
    s->m = 0;
    s->i = 0;
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
    
    for(; s->i < (s->size -1); s->i++) {
        if( (p[s->i] == 0xFF) && jif_is_marker_byte(p[s->i+1]) ) {   /* find a marker */
            s->m = ++s->i;
            return true;
        }
    }
    return false;
}

JIF_MARKER jif_get_current_marker(JIF_SCANNER * s){
    return s->pjif[s->m];
}

bool jif_scan_next_maker_of(JIF_MARKER m, JIF_SCANNER * s ){
    
    while ( jif_scan_next_marker(s) ){
        if (m == jif_get_current_marker(s)){
            return true;
        }
    }
    return false;
}

byte jif_scan_next_byte(JIF_SCANNER * s){
    return s->pjif[++s->i];
}

uint16_t jif_scan_2_bytes(JIF_SCANNER * s){
    return (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
}

uint32_t jif_scan_4_bytes(JIF_SCANNER * s){
    return  (jif_scan_next_byte(s) << 24) + ( jif_scan_next_byte(s) << 16 )
    + ( jif_scan_next_byte(s) << 8 ) + ( jif_scan_next_byte(s) );
}
