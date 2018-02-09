//
//  jquantdec.h
//  Mangamix
//
//  Created by theme on 09/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jquantdec_h
#define jquantdec_h

#include <stdio.h>
#include "jtypes.h"
#include "jentropydec.h"

/* DCT tables */
#define DCTSIZE     64
#define DCTSIZE_ROOT    8

typedef uint8_t coeff_t;

typedef union QUANT_TABLE_ARRAY {
    uint8_t     Q8[DCTSIZE];
    uint16_t    Q16[DCTSIZE];
} qtbl_arr;

typedef struct QUANT_TABLE {
    uint8_t     precison;       /* 8 | 16 */
    qtbl_arr    coeff_a;        /* DCT coeff array in zig-zag order */
} JTBL_QUANT;

void jquant_dequant(JTBL_QUANT * tQ, coeff_t * ZZ);

#endif /* jquantdec_h */
