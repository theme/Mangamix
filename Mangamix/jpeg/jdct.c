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


void j_idct_ZZ(coeff_t IDCT[DCTWIDTH][DCTWIDTH], coeff_t *ZZ, double helper_cos_arr[DCTWIDTH][DCTWIDTH]){
    uint16_t y, x;
    uint16_t v, u;
    
    double s, sum;
    
    for (y=0; y<DCTWIDTH; y++) {
        for (x=0; x<DCTWIDTH; x++) {
            
            sum = 0;
            for( u=0; u< DCTWIDTH; u++){
                for( v=0; v < DCTWIDTH; v++){
                    
                    s = (CALC_T)1.0;
                    if ( 0 == u ){
                        s /= sqrt(2);
                    }
                    if ( 0 == v ){
                        s /= sqrt(2);
                    }
                    s *= ZZ[ZZi64[v*8 + u]];
                    s *= helper_cos_arr[x][u];
                    s *= helper_cos_arr[y][v];
                    sum += s;
                }
            }
            
            IDCT[y][x] = sum / 4;
        }
    }
}

void j_ZZ_dbg(coeff_t *ZZ){
    // Debug
    coeff_t dbg[8][8];
    for(int j=0; j< 8; j++){
        for (int i = 0; i < 8 ; i++){
            dbg[j][i] = ZZ[ZZi64[j*8 + i]];
        }
    }
    return;
}
