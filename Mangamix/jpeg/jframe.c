//
//  jframe.c
//  Mangamix
//
//  Created by theme on 22/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//
//  part of decoder : decode a frame.

#include "jframe.h"

bool is_known_sof(JIF_MARKER m){
    switch (m) {
        case M_SOF0:
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
            return true;
        default:
            return false;
    }
}

bool jframe_read_jif(J_FRAME * f, JIF_SCANNER * s){
    int c;
    byte b;
    if( is_known_sof(jif_get_current_marker(s)) ){
        f->Lf = jif_scan_2_bytes(s);
        f->P = jif_scan_next_byte(s);
        f->Y = jif_scan_2_bytes(s);
        f->X = jif_scan_2_bytes(s);
        f->Nf = jif_scan_next_byte(s);
        for(c = 0; c < f->Nf; c++){
            f->comps[c].C = jif_scan_next_byte(s);
            b = jif_scan_next_byte(s);
            f->comps[c].H = ( b >> 4 );
            f->comps[c].V = ( 0x0f & b );
            f->comps[c].Tq = jif_scan_next_byte(s);
        }
        return true;
    }
    return false;
}


