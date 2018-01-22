//
//  jframe.h
//  Mangamix
//
//  Created by theme on 22/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jframe_h
#define jframe_h

#include <stdio.h>
#include "jif.h"


typedef struct {
    unsigned C;    /* Component identifier */
    unsigned H;     /* 1 ~ 4, Horizontal sampling factor: (component horizontal dimension) / X */
    unsigned V;     /* 1 ~ 4, Vertical sampling factor: (component vertical dimension) / Y */
    unsigned Tq;    /* Quantization table destination selector */
} J_COMPONENT;

typedef struct {
    unsigned Lf;    /* Frame header length */
    unsigned P;     /* Sample precision */
    unsigned Y;     /* max Number of lines in the source image */
    unsigned X;     /* max Number of samples per line */
    unsigned Nf;    /* Number of image components in frame (1: gray, 3:YCbCr|YIQ, 4:CMYK) */
    J_COMPONENT comps[4];     /* pointer to array of SOF_COMP */
} J_FRAME;

bool jframe_read_jif(J_FRAME * f, JIF_SCANNER * s);   /* if current marker is sof */


#endif /* jframe_h */
