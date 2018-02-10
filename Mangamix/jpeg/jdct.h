//
//  jdct.h
//  Mangamix
//
//  Created by theme on 10/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jdct_h
#define jdct_h

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "jquantdec.h"


#define DCTSIZE     64
#define DCTWIDTH    8

const uint16_t ZZi64[64] =
{  0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63};

void j_idct_ZZ(double IDCT[DCTWIDTH][DCTWIDTH], coeff_t *ZZ);

#endif /* jdct_h */
