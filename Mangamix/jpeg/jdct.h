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

#define DCTSIZE     64
#define DCTWIDTH    8

typedef uint16_t coeff_t;


void j_idct_ZZ(double IDCT[DCTWIDTH][DCTWIDTH], coeff_t *ZZ);

#endif /* jdct_h */
