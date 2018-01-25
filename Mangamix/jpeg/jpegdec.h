//
//  jpegdec.h
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

//  JPEG decoder : decode (jpeg.baseline) jif.

//  Input:
//      jif bits array. (jif file)
//  Output:
//      1. image info : with, height, color space, etc.
//      2. image pixel array (RGBA pixel).
//  Error:
//      defined in J_ERR.

/*
 *  To be used with osx Quartz 2D api (where a bitmap image is an
 *  array of pixels at a specific resolution)
 *  "The image data you supply to the function CGImageCreate must be interleaved
 *  on a per pixel, not a per scan line, basis. Quartz does not support planar
 *  data."
 *  e.g.{ R,G,B, R,G,B... }
 *
 *  ref: [Bitmap Images and Image
 *  Masks][https://developer.apple.com/library/content/documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/dq_images/dq_images.html#//apple_ref/doc/uid/TP30001066-CH212-TPXREF101]
 */

/*  This file is decoder's API. (APP should only need to include this file only.)
 *
 *  Overview of usage:
 *  1. j_dec_new() => (struct *) dinfo
 *  2. j_dec_set_src(src, dinfo)
 *  3. j_dec_read_header(dinfo)
 *      > j_info_get_width(dinfo)    => int w
 *      > j_info_get_height(dinfo)   => int h
 *      > j_info_get_colorspace(dinfo) => J_COLOR_SPACE J_CS_GRAY | _RGB  | _CMYK
 *      > j_info_get_components(dinfo) => int numbrer_of_components
 *      > j_info_get_component_depth(int c, dinfo)  => int c_depth
 *  4. j_dec_decode(dinfo) => true | false
 *      > j_info_get_error(dinfo)   => J_ERR ERR_XXXX
 *  5. j_dec_get_image(dinfo) => JSAMPLE[][] image_data
 *  6. j_dec_destroy(dinfo)
 */

#ifndef jpegdec_h
#define jpegdec_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct J_DEC_INFO *pinfo;

typedef enum {
    ERR_MALLOC,  /* not enough memory */
    ERR_UNKNOWN
} J_ERR;

typedef enum {
    J_COLOR_GRAY,
    J_COLOR_RGB,
    J_COLOR_CMYK
} J_COLOR_SPACE;    // TODO ?

pinfo j_dec_new(void);
bool j_dec_set_src_array(unsigned char *src, unsigned long long size, pinfo dinfo);
bool j_dec_read_header(pinfo dinfo);    /* read a frame in image */

unsigned long j_info_get_width(pinfo dinfo);
unsigned long j_info_get_height(pinfo dinfo);
J_COLOR_SPACE j_info_get_colorspace(pinfo dinfo);
int j_info_get_num_of_components(pinfo dinfo);
int j_info_get_component_depth(int comp_i, pinfo dinfo);    /* depth of i th component */

void * j_info_get_img_data(pinfo dinfo);
size_t j_info_get_img_data_size(pinfo dinfo);
void j_info_release_img_data(void *info, const void *data, size_t size);

bool j_dec_decode(pinfo dinfo);
J_ERR j_info_get_error(pinfo dinfo);
void j_dec_destroy(pinfo dinfo);


#endif /* jpegdec_h */
