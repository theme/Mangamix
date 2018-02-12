//
//  jentropydec.c
//  Mangamix
//
//  Created by theme on 27/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jpegdec.h"

JTBL_HUFF * jhuff_new(void){
    JTBL_HUFF * p = malloc(sizeof(JTBL_HUFF));
    p->lastk = 0;
    return p;
}
void jhuff_set_val(JTBL_HUFF * fh, huffindex_t k, huffval_t v){
    fh->huffval[k] = v;
}

void jhuff_gen_huffsize(JTBL_HUFF * th){
    uint16_t k = 0, i, j;
    
    for( i = 1; i <= HUFFCAT; i++ ){
        for ( j = 1; j <= th->bits[i]; j++){
            th->huffsize[k++] = i;
        }
    }
    
    th->huffsize[k] = 0;
    th->lastk = k;
}


void jhuff_gen_huffcode(JTBL_HUFF * th){
    uint16_t k = 0, code = 0, si = th->huffsize[0];
    
    while (true) {
        do {
            th->huffcode[k] = code;
            code += 1;
            k += 1;
        } while( th->huffsize[k] == si );
        
        if(0 == th->huffsize[k]){
            return;
        }
        
        do {
            code = code << 1;
            si += 1;
        } while ( th->huffsize[k] != si );
    }
}

void jhuff_gen_size_code(JTBL_HUFF * th){
    jhuff_gen_huffsize(th);
    jhuff_gen_huffcode(th);
    //    jhuff_gen_huffval(th);
}

void jhuff_gen_decode_tbls(JTBL_HUFF * th){
    for( int i = 1, j = 0; i <= HUFFCAT ; i++ ){
        if( 0 == th->bits[i] ){
            th->maxcode[i] = -1;
            continue;
        } else {
            th->valptr[i] = j;
            th->mincode[i] = th->huffcode[j];
            j += th->bits[i] - 1;
            th->maxcode[i] = th->huffcode[j];
            j++;
        }
    }
}

JERR nextbit(JIF_SCANNER * s){
    byte b2;
    if ( 0 == s->bit_cnt){
        s->bit_B = jif_scan_next_byte(s);
        s->bit_cnt = 8;
        if ( 0xFF == s->bit_B ){
            b2 = jif_scan_next_byte(s);
            if ( 0x00 != b2 ){
                if ( M_DNL == b2 ){
                    return JERR_HUFF_NEXTBIT_DNL;
                } else {
                    return JERR_UNKNOWN;
                }
            }
        }
    }
    
    s->bit_nextbit = s->bit_B >> 7;
    s->bit_cnt--;
    s->bit_B <<= 1;
    return JERR_NONE;
}

JERR jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s, huffsize_t *t){
    
    int i = 1;
    
    JERR e = nextbit(s);
    
    if ( JERR_NONE != e ){
        return e;
    }
    
    int16_t c = s->bit_nextbit;
    
    while( c > th->maxcode[i] ){
        i++;
        
        e = nextbit(s);
        if ( JERR_NONE != e ){
            return e;
        }
        
        c = (c << 1) + s->bit_nextbit;
    }
    
    huffindex_t j = th->valptr[i];
    j += c - th->mincode[i];
    *t = th->huffval[j];
    
    return  JERR_NONE;
}


JERR jhuff_receive(huffsize_t t, JIF_SCANNER * s, huffval_t * v){
    if ( 0 == t){
        return JERR_UNKNOWN;
    }
    
    /* jif scan should based on byte,
     * let jhuff_receive based on NEXTBIT, which handle EOB.
     */
    
    *v = 0x00;
    JERR e = JERR_NONE;
    for ( int i = 0; i < t; i ++ ){
        if ( JERR_NONE != (e = nextbit(s))) {
            break;
        }
        
        *v <<= 1;
        *v += s->bit_nextbit;
    }
    
    return e;
}

huffval_t jhuff_extend(huffval_t v, huffsize_t t) {
    huffval_t vt = 1 << ( t - 1 );
    
    if ( v < vt ){          // when v is negative, it's a negative DIFF/AC value
        vt = (-1 << t) + 1;         // is shifting a negative undefined ?
        vt = (0xffff << t) + 1;     // is this expected effect ?
        v += vt;
    }
    
    return v;
}

void jhuff_free(JTBL_HUFF * th){
    free(th->huffcode);
    free(th->huffsize);
    free(th->huffval);
    free(th);
}
