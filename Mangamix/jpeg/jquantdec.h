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
#include "jdct.h"

/* DCT tables */

typedef struct QUANT_TABLE {
    uint8_t     precison;       /* 8 | 16 */
    coeff_t    coeff_a[DCTSIZE];   /* DCT coeff array in zig-zag order */
} JTBL_QUANT;

void jquant_dequant(JTBL_QUANT * tQ, coeff_t * ZZ);


#endif /* jquantdec_h */
