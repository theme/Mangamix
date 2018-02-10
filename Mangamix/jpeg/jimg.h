//
//  jimg.h
//  Mangamix
//
//  Created by theme on 26/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jimg_h
#define jimg_h

// Image <= n x Components
//      Components <= sample matrix


//  Image (Source encoder | Reconstructed from decoder)
//  (0~255) components (sample arrays).

#include <stdio.h>
#include <stdlib.h>
#include "jtypes.h"

#define JMAX_COMPONENTS     256

typedef uint16_t    JIMG_SAMPLE;        /* enough to save 8 or 12 bit precision integer */

typedef struct {
    uint16_t    H;      /* sampling factor */
    uint16_t    V;      /* sampling factor */
    unsigned int  sample_precision;          /* DCT: 8,12; those lossless: 2 ~ 16 */
    JIMG_SAMPLE    *   data;                  /* the decoded image data */
} JIMG_COMPONENT;


typedef struct {
    uint16_t    X;      /* maximum component's width */
    uint16_t    Y;
    JIMG_COMPONENT  *comps[JMAX_COMPONENTS];
    bool        comp_map[JMAX_COMPONENTS];
    uint8_t     num_of_components;
} JIMG;

/* the output format */

typedef enum  {
    J_COLOR_GRAY,
    J_COLOR_RGB,
    J_COLOR_CMYK
} JIMG_COLOR_SPACE;

typedef struct {
    uint16_t    width;
    uint16_t    height;
    JIMG_COLOR_SPACE   color_space;
    uint8_t     bits_per_pixel;
    uint8_t     bits_per_component;
    byte        *data;
    size_t      data_size;
} JIMG_BITMAP;

JIMG * jimg_new(void);
JIMG * jimg_set_components(JIMG * img, uint8_t index, uint16_t X, uint16_t Y, unsigned int precision);
JIMG * jimg_write_sample(JIMG * img, uint8_t index, uint16_t x, uint16_t y, double s);
void jimg_free(JIMG * img);

JIMG_BITMAP * jbmp_new(void);
void jbmp_free(JIMG_BITMAP * bmp);

#endif /* jimg_h */
