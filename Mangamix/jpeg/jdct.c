//
//  jdct.c
//  Mangamix
//
//  Created by theme on 10/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jdct.h"

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
                        s *= 1 / sqrt(2);
                    }
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
