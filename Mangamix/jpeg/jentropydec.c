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

void generate_size_table(JTBL_HUFF * th){
    huffindex_t k = 0, j;
    huffsize_t i;
    
    for( i = 1; i <= HUFFCAT; i++ ){
        for ( j = 1; j <= th->bits[i]; j++){
            th->huffsize[k++] = i;
        }
    }
    
    th->huffsize[k] = 0;
    th->lastk = k;
}


void generate_code_table(JTBL_HUFF * th){
    huffindex_t k = 0;
    huffcode_t code = 0;
    huffsize_t si = th->huffsize[0];
    
    while (true) {
        do {
            th->huffcode[k] = code;
            code += 1;  /* generated huffcode list is in increasing order */
            k += 1;
        } while( th->huffsize[k] == si );
        
        if(0 == th->huffsize[k]){
            return;
        }
        
        do {
            code = code << 1; /* int16_t is signed. Left shift is always logical. */
            si += 1;
        } while ( th->huffsize[k] != si );
    }
}

void jhuff_gen_decode_tbls(JTBL_HUFF * th){
    generate_size_table(th);
    generate_code_table(th);
    /* when decoidng, huffval is read from DHT segment */
    
    for( huffindex_t i = 1, j = 0; i <= HUFFCAT ; i++ ){
        if( 0 == th->bits[i] ){
            th->maxcode[i] = -1;    /* sets all 1 (signed type) */
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

JERR jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s, huffval_t *t){
    huffindex_t i = 1;
    
    JERR e = nextbit(s);
    if ( JERR_NONE != e ){
        return e;
    }
    
    huffcode_t code = s->bit_nextbit;
    
    while( code > th->maxcode[i] ){
        i++;
        
        e = nextbit(s);
        if ( JERR_NONE != e ){
            return e;
        }
        
        code = (code << 1) + s->bit_nextbit;
    }
    
    huffindex_t j = th->valptr[i];
    j += code - th->mincode[i];
    *t = th->huffval[j];    /* choose value according to huff code */
    
    return  JERR_NONE;
}

/* put t bits into lower bits of coeff_t */
JERR jhuff_receive(huffsize_t t, JIF_SCANNER * s, coeff_t * v){
    JERR e = JERR_NONE;
    
    if ( 0 == t){
        *v = 0;
        return e;
    }

    for ( huffsize_t i = 0; i < t; i ++ ){
        if ( JERR_NONE != (e = nextbit(s))) {
            break;
        }
        
        *v <<= 1; /* Left shift is always logical. */
        *v += s->bit_nextbit;
    }
    
    return e;
}

/* extend coefficient to full precision */
coeff_t jhuff_extend(coeff_t v, huffsize_t t) {
    coeff_t vt = 1 << ( t - 1 );
    
    if ( v < vt ){
        vt = ((-1) << t) + 1;   // All 1 shift left
        v += vt;                // plus lower bit
    }
    
    return v;    /* extended to  precision t */
}

void jhuff_free(JTBL_HUFF * th){
    free(th->huffcode);
    free(th->huffsize);
    free(th->huffval);
    free(th);
}
