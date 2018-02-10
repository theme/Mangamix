//
//  jentropydec.c
//  Mangamix
//
//  Created by theme on 27/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jentropydec.h"


/* Huffman table generated from JIF file */

typedef struct {
    huff_size       size;       /* HUFFSIZE : size of each haffman code (1~16) */
    huff_code       code;       /* HUFFCODE */
    huff_val        value;
} huff_v;

struct _jtbl_huff {
    unsigned int    lastk;
    huff_v          *v;
    huff_code       mincode[HUFFCAT];
    huff_code       maxcode[HUFFCAT];
    huff_index      valptr[HUFFCAT];
};

JTBL_HUFF * jhuff_new(void){
    JTBL_HUFF * p = malloc(sizeof(JTBL_HUFF));
    p->lastk = 0;
    p->v = malloc(0);
    return p;
}

JIF_HUFF * jif_huff_new(void){
    JIF_HUFF * fh = malloc(sizeof(JIF_HUFF));
    fh->huffval = malloc(128);
    fh->huffval_capa = 128;
    return fh;
}
void jif_huff_set_val(JIF_HUFF * fh, huff_index i, huff_val v){
    if( i >= fh->huffval_capa ){
        fh->huffval = realloc(fh->huffval, sizeof(huff_val) * i *2);
        fh->huffval_capa = sizeof(huff_val) * i *2;
    }
    fh->huffval[i] = v;
}

/* expand table size if smaller than i */
bool touch_i(JTBL_HUFF * th, huff_index i){
    if( th->lastk < i) {
        th->v = realloc(th->v, 2 * i * sizeof(huff_v));
    }
    
    if (NULL != th->v)
        return true;
    else
        return false;
}

void jhuff_set_size(JTBL_HUFF * th, huff_index i, huff_size s){
    touch_i(th, i);
    th->v[i].size = s;
}

void jhuff_set_code(JTBL_HUFF * th, huff_index i, huff_code c){
    touch_i(th, i);
    th->v[i].code = c;
}

void jhuff_set_val(JTBL_HUFF * th, huff_index i, huff_val v){
    touch_i(th, i);
    th->v[i].value = v;
}

void jhuff_gen_huffsize(JIF_HUFF * fh, JTBL_HUFF * th){
    uint16_t k = 0, i, j;
    
    for( i = 1; i <= HUFFCAT; i++ ){
        for ( j = 1; j <= fh->bits[i]; j++){
            jhuff_set_size(th, k++, i);
        }
    }
    
    jhuff_set_size(th,k,0);
    th->lastk = k;
}


void jhuff_gen_huffcode(JIF_HUFF * fh, JTBL_HUFF * th){
    uint16_t k = 0, code = 0, si = th->v[0].size;
    
    while (true) {
        do {
            jhuff_set_code(th, k, code);
            code += 1;
            k += 1;
        } while( th->v[k].size == si );
        
        if(0 == th->v[k].size){
            return;
        }
        
        do {
            code = code << 1;
            si += 1;
        } while ( th->v[k].size != si );
    }
}

void jhuff_gen_huffval(JIF_HUFF * fh, JTBL_HUFF * th){
    uint16_t k = 0, i, j;
    
    for( i = 1; i <= HUFFCAT; i++ ){
        for ( j = 1; j <= fh->bits[i]; j++){
            jif_huff_set_val(fh, k, fh->huffval[k]);
            k++;
        }
    }
}

void jhuff_gen_decode_tbls(JIF_HUFF * fh, JTBL_HUFF * th){
    for( int i = 0, j = 0; i <= HUFFCAT ; i++ ){
        if( 0 == fh->bits[i] ){
            th->maxcode[i] = -1;
            continue;
        } else {
            th->valptr[i] = j;
            th->mincode[i] = th->v[j].code;
            j = j + fh->bits[i] -1;
            th->maxcode[i] = th->v[j].code;
            j++;
        }
    }
}


JTBL_HUFF * jtbl_huff_from_jif(JIF_HUFF * fh){
    JTBL_HUFF * th = jhuff_new();
    jhuff_gen_huffsize(fh, th);
    jhuff_gen_huffcode(fh, th);
    jhuff_gen_huffval(fh, th);
    jhuff_gen_decode_tbls(fh, th);
    return th;
}

huff_size jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s){
    
    int i = 1;
    
    huff_code c = jif_scan_next_bit(s);
    
    while( c > th->maxcode[i] ){
        i++;
        c = (c << 1) + jif_scan_next_bit(s);
        
    }
    
    huff_index j = th->valptr[i];
    j = j + c - th->mincode[i];
    return th->v[j].value;
}

huff_val jhuff_receive(huff_size t, JIF_SCANNER * s){
    if ( 0 == t){
        return 0;
    }
    
    return jif_scan_t_bits(s, t);
}

huff_val jhuff_extend(huff_val v, huff_size t) {
    huff_val vt = 1 << ( t - 1 );
    
    if ( v < vt ){          // when v is negative, it's a negative DIFF/AC value
        vt = (-1 << t) + 1;         // is shifting a negative undefined ?
        vt = (0xffff << t) + 1;     // is this expected effect ?
        v += vt;
    }
    
    return v;
}

void jhuff_free(JTBL_HUFF * th){
    free(th->v);
    free(th);
}

void jif_huff_free(JIF_HUFF * fh){
    free(fh->huffval);
    free(fh);
}