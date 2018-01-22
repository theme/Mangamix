//
//  jimage.h
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//
//  Raw image. 

#ifndef jimage_h
#define jimage_h


typedef enum {
    J_COLOR_GRAY,
    J_COLOR_RGB,
    J_COLOR_CMYK
} J_COLOR_SPACE;    // TODO ?

typedef unsigned int uint;

typedef struct {
    unsigned int width;    /* jpeg usual size : 1 ~ 65535 */
    unsigned int height;
    unsigned int num_of_components;
    J_COLOR_SPACE color_space;
    uint bits_per_pixel;
    uint bits_per_component;    /* TODO ? In decoded raw image, each component has same depth. */
} J_IMAGE;

#endif /* jimage_h */
