//
//  jdct.c
//  Mangamix
//
//  Created by theme on 10/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jdct.h"

const uint16_t ZZi64[64] =
{  0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63};

coeff_t uv_of_ZZ64(uint16_t v, uint16_t u, coeff_t *ZZ){
    return ZZ[ZZi64[v*8 + u]];
}

void j_idct_ZZ(double IDCT[DCTWIDTH][DCTWIDTH], coeff_t *ZZ){
    uint16_t y, x;
    uint16_t v, u;
    
    double    s;
    
    for (y=0; y<DCTWIDTH; y++) {
        for (x=0; x<DCTWIDTH; x++) {
            
            s = 1.0;
            
            for( v=0; v < DCTWIDTH; v++){
                for( u=0; u< DCTWIDTH; u++){
                    
                    if ( 0 == u && 0 == v){
                        s *= 1 / 2;
                    }
                    s *= uv_of_ZZ64(v, u, ZZ);
                    s *= cos(2*x + 1) * u * M_PI;
                    s *= cos(2*y + 1) * v * M_PI;
                    s /= 16;
                    s /= 16;
                }
            }
            
            IDCT[y][x] = s / 4.0;
        }
    }
}
