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
    news->pjif = s->pjif;
    news->size = s->size;
    news->m = s->m;
    news->i = s->i;
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

bool jif_read_frame_param(JIF_SCANNER * s) {
    int c;
    byte b;
    JIF_MARKER m = jif_get_current_marker(s);
    switch (m) {
        case M_SOF0:
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
            s->frame.Lf = (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
            s->frame.P = jif_scan_next_byte(s);
            s->frame.Y = (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
            s->frame.X = (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
            s->frame.Nf = jif_scan_next_byte(s);
            for(c = 0; c < s->frame.Nf; c++){
                s->frame.comps[c].C = jif_scan_next_byte(s);
                b = jif_scan_next_byte(s);
                s->frame.comps[c].H = ( b >> 4 );
                s->frame.comps[c].V = ( 0x0f & b );
                s->frame.comps[c].Tq = jif_scan_next_byte(s);
            }
            break;
            
        default:
            break;
    }
    return true;
}

JIF_MARKER jif_get_current_marker(JIF_SCANNER * s){
    return s->pjif[s->m];
}


