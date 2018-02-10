//
//  jquantdec.c
//  Mangamix
//
//  Created by theme on 09/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jquantdec.h"

void jquant_dequant(JTBL_QUANT * tQ, coeff_t * ZZ){
    for( int i = 0; i < DCTSIZE; i++){
        ZZ[i] *= tQ->coeff_a[i];
    }
}
