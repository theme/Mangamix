//
//  jtablespec.h
//  Mangamix
//
//  Created by theme on 27/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jtablespec_h
#define jtablespec_h

#include <stdio.h>
#include <stdbool.h>
#include "jtypes.h"

/* DCT tables */
#define DCTSIZE     64

typedef union QUANT_TABLE_ARRAY {
    uint8_t     Q8[DCTSIZE];
    uint16_t    Q16[DCTSIZE];
} qtbl_arr;

typedef struct QUANT_TABLE {
    uint8_t     precison;       /* 8 | 16 */
    qtbl_arr    coeff_a;        /* DCT coeff array in zig-zag order */
} JTBL_QUANT;

/* Huffman tables */
typedef struct {
    bool        Tc;             /* (Table class) 0: DC | lossless table, 1: AC table */
    byte        nL[16];          /* Number of Huffman codes of length i (0~255) */
    uint8_t     *vArrays[16];     /* 16 array pinter, to 16 list saving values of length i(0~255) */
} JTBL_HUFF;


#endif /* jtablespec_h */
