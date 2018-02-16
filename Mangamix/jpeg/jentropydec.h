//
//  jentropydec.h
//  Mangamix
//
//  Created by theme on 16/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jentropydec_h
#define jentropydec_h

#include "jerr.h"


/********************************************************
 Entropy coding routine
 ********************************************************/

/* Huffman table in JIF file */

#define HUFFCAT    16

typedef uint16_t huffindex_t;   /* to contain 0 ~ cat * 256 value */
typedef byte    huffval_t;      /* type that contains a huffman value */
typedef uint8_t huffsize_t;     /* type that contains a huffman code length in bit */
typedef int16_t huffcode_t;    /* a huffman code (1 ~ 16 bit) */

typedef struct {
    /* given ( from JIF ) */
    short int       table_class;            /* (Table class) 0: DC | lossless table, 1: AC table */
    huffsize_t         bits[HUFFCAT+1];        /* Number (0~255) of Huffman codes for code length i (0~16) */
    huffval_t         huffval[HUFFCAT*256];    /* HUFFVAL : symbol values list (in order of increasing code length.)
                                                * B.2.4.2 (The meaning of each value is determined by the Huffman coding model)
                                                */
    /* generated ( for decoding ) */
    huffsize_t         huffsize[HUFFCAT*256];  /* HUFFSIZE : a list of code length */
    huffcode_t        huffcode[HUFFCAT*256];  /* HUFFCODE : huffman codes corresponding to above length */
    huffindex_t        lastk;                  /* last code index of HUFFCODE */
    huffcode_t         mincode[HUFFCAT+1];     /* (F.2.2.3 ) signed 16-bit integers for convenience of –1 sets all of the bits. (... guaranteed by C99 standard to be portable regardless of the signed number representation of the system ) */
    huffcode_t         maxcode[HUFFCAT+1];
    huffindex_t        valptr[HUFFCAT+1];
} JTBL_HUFF;

#define HUFFVAL_INIT_SIZE 256

/* basic */
JTBL_HUFF * jhuff_new(void);
void jhuff_set_val(JTBL_HUFF * fh, huffindex_t k, huffval_t v);

JTBL_HUFF * jhuff_new(void);

/* before decoding */
void jhuff_gen_decode_tbls(JTBL_HUFF * th);
/* decode */
JERR jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s, huffval_t * t);
JERR jhuff_receive(huffsize_t t, JIF_SCANNER * s, huffval_t * v); /* places the next T bits of the serial bit string into the low order bits of DIFF */
huffval_t jhuff_extend(huffval_t src, huffsize_t t); /* converts the partially decoded DIFF value of precision T to the full precision difference */

/* cleanup */
void jhuff_free(JTBL_HUFF *);



#endif /* jentropydec_h */
