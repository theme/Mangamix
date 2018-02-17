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
#include "jtypes.h"


/********************************************************
 Entropy coding routine
 ********************************************************/

/* Huffman table in JIF file */

#define HUFFCAT    16

typedef uint16_t huffindex_t;   /* to contain 0 ~ cat * 256 value */
typedef uint8_t huffval_t;      /* defined in DHT marker, 8 bit */
typedef uint8_t huffsize_t;     /* a huffman code's bit length */
typedef int16_t huffcode_t;    /* a huffman code (1 ~ 16 bits) */

typedef struct {
    /* given ( from JIF ) */
    short int       table_class;            /* (Table class) 0: DC | lossless table, 1: AC table */
    huffsize_t         bits[HUFFCAT+1];        /* Count (0~255) of Huffman codes for each code length i (0~16) */
    huffval_t         huffval[HUFFCAT*256];    /* HUFFVAL : symbol values list (in order of increasing code length.)
                                                * B.2.4.2 (The meaning of each value is determined by the Huffman coding model)
                                                */
    /* generated ( for decoding ) */
    huffsize_t         huffsize[HUFFCAT*256];  /* HUFFSIZE : list of each code length */
    huffcode_t         huffcode[HUFFCAT*256];  /* HUFFCODE : list of each code */
    huffindex_t        lastk;                  /* last code index in the list */
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

/* Decode a huffman encoded value using huffman table, value list is defined along
 * with DHT table int the DHT marker segment ( is 8 bit value ).
 */
JERR jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s, huffval_t * t);

 /* Places the next T bits of the serial bit string into the low order bits of v. for DC, this is DIFF's lower bits , for AC this is the AC coefficient's lower bits. */
JERR jhuff_receive(huffsize_t t, JIF_SCANNER * s, coeff_t * v);

 /* Extend the partially decoded DIFF/AC value of precision T to the full precision */
coeff_t jhuff_extend(coeff_t src, huffsize_t t);

/* cleanup */
void jhuff_free(JTBL_HUFF *);



#endif /* jentropydec_h */
