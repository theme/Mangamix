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

/*  This file is jif decoder's API. (decode jif to img components. Post convertion to bmp is in jimg.h)
 *
 *  Overview of usage:
 *  1. j_dec_new() => (struct *) dinfo
 *  2. j_dec_set_src(src, dinfo)
 *  3. j_dec_read_header(dinfo)
 *      > j_info_get_width(dinfo)    => int w
 *      > j_info_get_height(dinfo)   => int h
 *      > j_info_get_components(dinfo) => int numbrer_of_components
 *  4. j_dec_decode(dinfo) => true | false
 *      > j_info_get_error(dinfo)   => J_ERR ERR_XXXX
 *  5. j_dec_get_RGB_image(dinfo) => uint8_t * image_data
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

#define err(S) fprintf(stderr, S)
#define logger(S) printf(S)


typedef enum {
    JERR_NONE = 0,
    JERR_MALLOC = -1,  /* not enough memory */
    JERR_UNKNOWN = -2,
    JERR_HUFF_NEXTBIT_DNL = -3,
    JERR_BAD_SCAN_HEADER = -4
} JERR;



 /********************************************************
  Entropy coding routine
  ********************************************************/

/* Huffman table in JIF file */

#define HUFFCAT    16

typedef uint16_t huffindex_t;
typedef uint8_t huffval_t;
typedef uint8_t huffsize_t;

typedef struct {
    /* given ( from JIF ) */
    short int       table_class;            /* (Table class) 0: DC | lossless table, 1: AC table */
    uint8_t         bits[HUFFCAT+1];        /* Number (0~255) of Huffman codes for code length i (0~16) */
    uint8_t         huffval[HUFFCAT*256];    /* HUFFVAL : symbol values list (in order of increasing code length.)
                                                 * B.2.4.2 (The meaning of each value is determined by the Huffman coding model)
                                                 */
    /* generated ( for decoding ) */
    uint8_t         huffsize[HUFFCAT*256];  /* HUFFSIZE : a list of code length */
    uint16_t        huffcode[HUFFCAT*256];  /* HUFFCODE : huffman codes corresponding to above length */
    uint16_t        lastk;                  /* last code index of HUFFCODE */
    int16_t         mincode[HUFFCAT+1];     /* (F.2.2.3 ) signed 16-bit integers for convenience of –1 sets all of the bits. (... guaranteed by C99 standard to be portable regardless of the signed number representation of the system ) */
    int16_t         maxcode[HUFFCAT+1];
    uint16_t        valptr[HUFFCAT+1];
} JTBL_HUFF;

#define HUFFVAL_INIT_SIZE 256

/* basic */
JTBL_HUFF * jhuff_new(void);
void jhuff_set_val(JTBL_HUFF * fh, huffindex_t k, huffval_t v);

JTBL_HUFF * jhuff_new(void);

/* before decoding */
void jhuff_gen_decode_tbls(JTBL_HUFF * th);
/* decode */
JERR jhuff_decode(JTBL_HUFF * th, JIF_SCANNER * s, huffval_t * t);
JERR jhuff_receive(huffsize_t t, JIF_SCANNER * s, huffval_t * v); /* places the next T bits of the serial bit string into the low order bits of DIFF */
huffval_t jhuff_extend(huffval_t src, huffsize_t t); /* converts the partially decoded DIFF value of precision T to the full precision difference */

/* cleanup */
void jhuff_free(JTBL_HUFF *);


/********************************************************
 Decoder
 ********************************************************/

 /* decoder status */
#define JTBL_NUM 4

typedef struct J_DEC_INFO {
    /* (per file) input: jif data */
    byte               *src;        /* source jif array (to be decoded) */
    jif_offset          src_size;
    
    /* (per file) output: image data */
    JIMG                *img;
    JBMP         *bmp;
    
    /* (per decoder) coding etc. */
    bool                is_dct_based;
    bool                is_use_arithmetic_coding;
    
    /* (per image) pointer to tables spec */
    JTBL_QUANT          tQ[JTBL_NUM];
    JTBL_HUFF           *tH[2][JTBL_NUM];   /* two class */
    
    /* (per frame) frame and component */
    JIF_FRAME_MODE      f_mode;     /* got from tables/misc. segments after SOI */
    JIF_FRAME           frame;
    
    /* (per scan)  */
    JIF_SCAN            scan;
    uint16_t            Ri;         /* restart interval */
    uint16_t            m;          /* in scan MCU counter */
    uint16_t            Nb;         /* # of data units in MCU
                                     (calculated from frame and scan) */
    
    /* (for decoder) other info */
    JERR           err;
} * pinfo;


pinfo j_dec_new(void);
bool j_dec_set_src_array(unsigned char *src, unsigned long long size, pinfo dinfo);
bool j_dec_read_jpeg_header(pinfo dinfo);    /* read a frame in image */

unsigned long j_info_img_width(pinfo dinfo);
unsigned long j_info_img_height(pinfo dinfo);
unsigned int j_info_get_components(pinfo dinfo);

bool j_dec_decode(pinfo dinfo);

JERR j_info_get_error(pinfo dinfo);
void j_dec_destroy(pinfo dinfo);

#endif /* jpegdec_h */
