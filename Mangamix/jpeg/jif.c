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

typedef struct jif_scanner {
    byte * pjif;
    jif_offset size;
    jif_offset cur_m; /* current marker */
    jif_offset old_m;
} JIF_SCANNER;


bool jif_is_marker(byte b) {
    return (b != '\x00') && (b !='\xFF'); /* Note all byte in open range (0x00, 0xff) are possible markers */
}


/* if (return < to), found a marker */
JIF_SCANNER * jif_new_scanner(byte * jif_array, jif_offset array_size){
    JIF_SCANNER * pscanner = (JIF_SCANNER *)malloc(sizeof(JIF_SCANNER));
    pscanner->pjif = jif_array;
    pscanner->size = array_size;
    pscanner->cur_m = 0;
    pscanner->old_m = 0;
    return pscanner;
}


JIF_SCANNER * jif_copy_scanner(JIF_SCANNER * pscanner){
    JIF_SCANNER * s = (JIF_SCANNER *)malloc(sizeof(JIF_SCANNER));
    s->pjif = pscanner->pjif;
    s->size = pscanner->size;
    s->cur_m = pscanner->cur_m;
    s->old_m = pscanner->old_m;
    return s;
}

void jif_del_scanner(JIF_SCANNER * pscanner){
    free(pscanner);
}

bool jif_scan_next_marker(JIF_SCANNER * scanner){
    jif_offset f;  /* will point to \xff */
    byte* p = scanner->pjif;    /* helper for shorter code */
    
    /* sacn for marker. \xFF is fewer in data, scan for it, then judge the rest. */
    for(f = scanner->cur_m ; f < (scanner->size -1); f++) {
        if( p[f] == '\xFF' && jif_is_marker(p[f+1])) {
            scanner->old_m = scanner->cur_m;    /* hold current index */
            scanner->cur_m = f+1;
            return true;
        }
    }
    return false;
}

bool jif_scan_next_maker_of(JIF_MARKER e_marker, JIF_SCANNER * scanner ){
    
    while ( (scanner->cur_m < scanner->size) && jif_is_marker(scanner->cur_m) ){
           if (e_marker == jif_get_current_marker(scanner)){
               return true;
           } else {
               jif_scan_next_marker(scanner);
           }
    }
    return false;
}

JIF_MARKER jif_get_current_marker(JIF_SCANNER * scanner){
    return scanner->pjif[scanner->cur_m];
}


