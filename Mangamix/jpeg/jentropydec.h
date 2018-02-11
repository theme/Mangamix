//
//  jentropydec.h
//  Mangamix
//
//  Created by theme on 27/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jentropydec_h
#define jentropydec_h

#include <stdio.h>
#include <stdlib.h>
#include "jtypes.h"
#include "jif.h"

#define HUFFCAT    16

typedef uint8_t huff_size;
typedef uint16_t huff_index;
typedef int16_t huff_code;  /* F.2.2.3 */
typedef uint8_t huff_val;   /* B.2.4.2 (The meaning of each value is determined by the Huffman coding model) */

/* Huffman table in JIF file */

typedef struct _jif_huff {
    bool        Tc;             /* (Table class) 0: DC | lossless table, 1: AC table */
    byte        bits[HUFFCAT];          /* Number (0~255) of Huffman codes of size (length) i (0~16) */
    huff_val    *huffval;       /* HUFFVAL */
    huff_index  huffval_capa;   /* realloc size of huffval */
} JIF_HUFF;

typedef struct _jtbl_huff JTBL_HUFF;

typedef enum {
    JHUFF_ERR_NONE = 0,
    JHUFF_ERR_DNL = -1,
    JHUFF_ERR_UNKNOWN = -2
} JHUFF_ERR;

/* basic */
JIF_HUFF * jif_huff_new(void);
void jif_huff_set_val(JIF_HUFF * fh, huff_index i, huff_val v);

JTBL_HUFF * jhuff_new(void);

/* user */
JTBL_HUFF * jtbl_huff_from_jif(JIF_HUFF * fh);

/* decode */
JHUFF_ERR jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s, huff_size * t);
JHUFF_ERR jhuff_receive(huff_size t, JIF_SCANNER * s, huff_val * v); /* places the next T bits of the serial bit string into the low order bits of DIFF */
huff_val jhuff_extend(huff_val src, huff_size t); /* converts the partially decoded DIFF value of precision T to the full precision difference */

/* cleanup */
void jhuff_free(JTBL_HUFF *);
void jif_huff_free(JIF_HUFF *);

#endif /* jentropydec_h */
