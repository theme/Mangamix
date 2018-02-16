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
    uint8_t     cid;       /* a unique label to the ith component */
    uint16_t    X;        
    uint16_t    Y;
    JIMG_SAMPLE **lines;    /* the decoded image data */
} JIMG_COMPONENT;

typedef struct {
    uint16_t        X;      /* maximum component's width */
    uint16_t        Y;
    unsigned int    precision;        /* DCT: 8,12; those lossless: 2 ~ 16 */
    JIMG_COMPONENT  *comps;
    int             comps_count;    /* number of component */
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
    uint16_t    bytes_per_row;
} JBMP_INFO;

/* construct */
JIMG * jimg_new(uint16_t width, uint16_t height, uint16_t precision);
JIMG_COMPONENT * jimg_set_component(JIMG * img, uint8_t comp_id, uint16_t width, uint16_t height);
JIMG_COMPONENT * jimg_get_component(JIMG * img, uint8_t comp_id);

/* returns 0 : dropped, else : wrote. */
JIMG * jimg_write_sample(JIMG * img, uint8_t comp_i, uint16_t x, uint16_t y, double s);

/* free */
void jimg_free(JIMG * img);

/* write out bmp to *dst */
void jbmp_make_RGBA32(JIMG * img, void * dst);

#endif /* jimg_h */
