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
//  Image Components may have different size. (But sampling factor is not kept here.)

#include <stdio.h>
#include <stdlib.h>
#include "jtypes.h"
#include "jcolor.h"

#define JMAX_COMPONENTS     256

typedef uint16_t    JIMG_SAMPLE;        /* enough to save 8 or 12 bit precision integer */

typedef struct _jimg_component JIMG_COMPONENT;

typedef struct _jimg JIMG;

/* construct */
JIMG * jimg_new(uint16_t width, uint16_t height, uint16_t precision);
uint16_t jimg_X(JIMG * img);
uint16_t jimg_Y(JIMG * img);
uint8_t jimg_component_count(JIMG * img);

JIMG_COMPONENT * jimg_set_component(JIMG * img, uint8_t comp_id, uint16_t width, uint16_t height);
JIMG_COMPONENT * jimg_get_component(JIMG * img, uint8_t comp_id);

/* returns 0 : dropped, else : wrote. */
JIMG * jimg_write_sample(JIMG * img, uint8_t comp_i, uint16_t x, uint16_t y, JIMG_SAMPLE s);
JIMG_SAMPLE jimg_get_sample(JIMG * img, uint8_t comp_i, uint16_t x, uint16_t y );

/* free */
void jimg_free(JIMG * img);


#endif /* jimg_h */
