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
#include "jentropydec.h"

/* DCT tables */
#define DCTSIZE     64
#define DCTSIZE_ROOT    8

typedef union QUANT_TABLE_ARRAY {
    uint8_t     Q8[DCTSIZE];
    uint16_t    Q16[DCTSIZE];
} qtbl_arr;

typedef struct QUANT_TABLE {
    uint8_t     precison;       /* 8 | 16 */
    qtbl_arr    coeff_a;        /* DCT coeff array in zig-zag order */
} JTBL_QUANT;



#endif /* jtablespec_h */
