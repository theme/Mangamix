//
//  jimg.h
//  Mangamix
//
//  Created by theme on 26/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jimg_h
#define jimg_h

//  Image (Source | Reconstructed)
//  (0~255) components (sample arrays).

#include <stdio.h>
#include "jtypes.h"

typedef struct  {
    unsigned int precision;     /* DCT: 8,12; those lossless: 2 ~ 16 */
} JIMG_COMP;

typedef enum  {
    J_COLOR_GRAY,
    J_COLOR_RGB,
    J_COLOR_CMYK
} JIMG_COLOR_SPACE;

typedef struct {
    uint16_t    width;    /* jpeg usual size : 1 ~ 65535 */
    uint16_t    height;
    uint8_t     num_of_components;
    JIMG_COLOR_SPACE   color_space;
    uint8_t     bits_per_pixel;
    uint8_t     bits_per_component;    /* TODO ? In decoded raw image, each component has same depth. */
    byte        *data;      /* the decoded image data */
    size_t      data_size;
} JIMG;

#endif /* jimg_h */
