//
//  jpegdec.h
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

//  JPEG decoder  (is embodiment of coding *process*) : decode (jpeg.baseline) jif.

//  Input:
//      jif bits array. (jif file)
//  Output:
//      Image (a set of components). (can be futher transformed into pixmap)
//      1. image info : with, height, color space, etc.
//      2. image components array.
//  Error:
//      defined in J_ERR.

/*
 *  The Baselien sequenatial process is DCT based, and use Huffman coding.
 *  Many jpeg files use this mode.
 */

/*  In standard : CCITT Rec. T.81 (1992 E) = ISO/IEC 10918-1 : 1993(E)
 *  "INFORMATION TECHNOLOGY –
 *      DIGITAL COMPRESSION AND CODING OF CONTINUOUS-TONE STILL IMAGES – REQUIREMENTS AND GUIDELINES"
 *  which was prepared by CCITT Study Group VIII and the Joint Photographic Experts Group (JPEG) of ISO/IEC JTC 1/SC 29/WG 10.
 *
 *  There are 2 class of encoding & decoding process.
 *      1. lossy    -   based on DCT : baseline sequential process / extended DCT-based process
 *      2. lossless -   not based on DCT
 *
 *  There are 4 distinct _modes of operation_ under wich various coding processes are defined.
 *      1. sequentail DCT based - 8 x 8 sample blocks are DCT transformed, quantized, entropy encoded, out putted.
 *      2. progressive DCT based - 8 x 8 sample blocks are DCT transformed, quantized,
 *                                  buffered, spectral selected , successive approximated, partially encoded in each of multiple scans.
 *      3. lossless
 *      4. hierarchical - multi-resolution requirements
 *
 *  There are 2 encoding procudure for _entropy encoding_:
 *      1. Huffman coding   (use Huffman tables, no default ones, APP may choose)
 *      2. arithmetic coding (use arithmetic coding conditioning tables, defined )
 */

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
 *
 *  Caveat: JIF format is specified, but is generated by an encoder, so the basic
 *  resonable components of decoder is similar to the encoder.
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

#include "jif.h"
#include "jimg.h"
#include "jquantdec.h"
#include "jentropydec.h"

#define err(S) fprintf(stderr, S)
#define logger(S) printf(S)

typedef enum {
    ERR_MALLOC,  /* not enough memory */
    ERR_UNKNOWN
} J_ERR;

 /* decoder status */
#define JTBL_NUM 4

typedef struct J_DEC_INFO {
    /* (per file) input: jif data */
    byte               *src;        /* source jif array (to be decoded) */
    jif_offset          src_size;
    
    /* (per file) output: image data */
    JIMG                *img;
    JIMG_BITMAP         *bmp;
    
    /* (per decoder) coding etc. */
    bool                is_dct_based;
    bool                is_use_arithmetic_coding;
    
    /* (per image) pointer to tables spec */
    JTBL_QUANT          tQ[JTBL_NUM];
    JTBL_HUFF           *tH[JTBL_NUM];
    JIF_HUFF            *jH[JTBL_NUM];
    
    /* (per frame) frame and component */
    JIF_FRAME_MODE      f_mode;     /* got from tables/misc. segments after SOI */
    JIF_FRAME           frame;
    
    /* (per scan)  */
    JIF_SCAN            scan;
    uint16_t            Ri;        /* scan restart interval */
    uint8_t             du_per_MCU;     /* data unit per MCU */
    uint16_t            MCU_per_scan;
    uint16_t            dec_MCU_i;
    
    /* (for decoder) other info */
    J_ERR           err;
} * pinfo;


pinfo j_dec_new(void);
bool j_dec_set_src_array(unsigned char *src, unsigned long long size, pinfo dinfo);
bool j_dec_read_header(pinfo dinfo);    /* read a frame in image */

unsigned long j_info_get_width(pinfo dinfo);
unsigned long j_info_get_height(pinfo dinfo);
JIMG_COLOR_SPACE j_info_get_colorspace(pinfo dinfo);
int j_info_get_num_of_components(pinfo dinfo);
int j_info_get_component_depth(int comp_i, pinfo dinfo);    /* depth of i th component */

void * j_info_get_bmp_data(pinfo dinfo);
size_t j_info_get_bmp_data_size(pinfo dinfo);

void j_info_release_bmp_data(void *info, const void *data, size_t size);

bool j_dec_decode(pinfo dinfo);
J_ERR j_info_get_error(pinfo dinfo);
void j_dec_destroy(pinfo dinfo);


#endif /* jpegdec_h */
