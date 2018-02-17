//
//  jcolor.c
//  Mangamix
//
//  Created by theme on 16/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jcolor.h"


uint8_t clamp( float v){
    if ( v < 0 ){
        return 0;
    }
    if ( v > 255 ){
        return 255;
    }
    
    return v;
}


uint32_t jYCbCrA2RGBA(uint32_t pixel){
    uint8_t Y = pixel >> 24;
    uint8_t Cb = (pixel >> 16) & 0xFF;
    uint8_t Cr = (pixel >> 8) & 0xFF;
    uint8_t A = pixel & 0xFF;
    
    uint8_t R = clamp(Y                           + 1.402 * (Cr - 128));
    uint8_t G = clamp(Y - 0.344136 * (Cb - 128)   - 0.714136 * (Cr - 128));
    uint8_t B = clamp(Y + 1.772 * (Cb - 128));
    
    pixel = R;
    pixel <<= 8;
    pixel += G;
    pixel <<= 8;
    pixel += B;
    pixel <<= 8;
    pixel += A;
    return pixel;
}
