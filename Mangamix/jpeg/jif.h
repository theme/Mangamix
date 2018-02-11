//
//  jif.h
//  Mangamix
//
//  Created by theme on 02/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//
//  JIF: JPEG Ingerchange format ( ref: JPEG - ISO/IEC 10918-1 : 1993 )

#ifndef jif_h
#define jif_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* memcpy() */

#include "jtypes.h"
#include "jdct.h"

typedef unsigned long long jif_offset;  /* offset in side array of target file */

typedef enum {
    /* Other markers */
    M_TEM = 0x01, /* For temporary private use in arithmetic coding */
    M_RESx02 = 0x02, /* Reserved */
    /* 0xff02 ~ 0xffbf : Reserved */
    M_RESxbf = 0xbf, /* Reserved */
    /* Start Of Frame markers, non-differential, Huffman coding */
    M_SOF0 = 0xc0, /* Baseline DCT */
    M_SOF1 = 0xc1, /* Extended sequential DCT */
    M_SOF2 = 0xc2, /* Progressive DCT */
    M_SOF3 = 0xc3,  /* Lossless (sequential) */
    /* Differential sequential DCT */
    M_DHT  = 0xc4, /* Define Huffman table(s) */
    /* Start Of Frame markers, differential, Huffman coding */
    M_SOF5 = 0xc5, /* Differential sequential DCT */
    M_SOF6 = 0xc6, /* Differential progressive DCT */
    M_SOF7 = 0xc7, /* Differential lossless (sequential) */
    /* Start Of Frame markers, non-differential, arithmetic coding */
    M_JPG  = 0xc8, /* Reserved for JPEG extensions */
    M_SOF9 = 0xc9, /* Extended sequential DCT */
    M_SOF10= 0xca, /* Progressive DCT */
    M_SOF11= 0xcb, /* Lossless (sequential) */
    /* Arithmetic coding conditioning specification */
    M_DAC  = 0xcc, /* Define arithmetic coding conditioning(s) */
    /* Differential sequential DCT */
    M_SOF13= 0xcd, /* Lossless (sequential) */
    M_SOF14= 0xce, /* Differential progressive DCT */
    M_SOF15= 0xcf, /* Differential lossless (sequential) */
    /* Restart interval termination */
    M_RST0 = 0xd0, /* Restart with modulo 8 count “0” */
    M_RST1 = 0xd1, /* Restart with modulo 8 count “1” */
    M_RST2 = 0xd2, /* Restart with modulo 8 count “2” */
    M_RST3 = 0xd3, /* Restart with modulo 8 count “3” */
    M_RST4 = 0xd4, /* Restart with modulo 8 count “4” */
    M_RST5 = 0xd5, /* Restart with modulo 8 count “5” */
    M_RST6 = 0xd6, /* Restart with modulo 8 count “6” */
    M_RST7 = 0xd7, /* Restart with modulo 8 count “7” */
    /* Other markers */
    M_SOI = 0xd8, /* Start of image */
    M_EOI = 0xd9, /* End of image */
    M_SOS = 0xda, /* Start of scan */
    M_DQT = 0xdb, /* Define quantization table(s) */
    M_DNL = 0xdc, /* Define number of lines */
    M_DRI = 0xdd, /* Define restart interval */
    M_DHP = 0xde, /* Define hierarchical progression */
    M_EXP = 0xdf, /* Expand reference component(s) */
    M_APP0 = 0xe0, /* Reserved for application segments */
    M_APP1 = 0xe1, /* Reserved for application segments */
    M_APP2 = 0xe2, /* Reserved for application segments */
    M_APP3 = 0xe3, /* Reserved for application segments */
    M_APP4 = 0xe4, /* Reserved for application segments */
    M_APP5 = 0xe5, /* Reserved for application segments */
    M_APP6 = 0xe6, /* Reserved for application segments */
    M_APP7 = 0xe7, /* Reserved for application segments */
    M_APP8 = 0xe8, /* Reserved for application segments */
    M_APP9 = 0xe9, /* Reserved for application segments */
    M_APP10 = 0xea, /* Reserved for application segments */
    M_APP11 = 0xeb, /* Reserved for application segments */
    M_APP12 = 0xec, /* Reserved for application segments */
    M_APP13 = 0xed, /* Reserved for application segments */
    M_APP14 = 0xee, /* Reserved for application segments */
    M_APP15 = 0xef, /* Reserved for application segments */
    M_JPG0 = 0xf0, /* Reserved for JPEG extensions */
    M_JPG1 = 0xf1, /* Reserved for JPEG extensions */
    M_JPG2 = 0xf2, /* Reserved for JPEG extensions */
    M_JPG3 = 0xf3, /* Reserved for JPEG extensions */
    M_JPG4 = 0xf4, /* Reserved for JPEG extensions */
    M_JPG5 = 0xf5, /* Reserved for JPEG extensions */
    M_JPG6 = 0xf6, /* Reserved for JPEG extensions */
    M_JPG7 = 0xf7, /* Reserved for JPEG extensions */
    M_JPG8 = 0xf8, /* Reserved for JPEG extensions */
    M_JPG9 = 0xf9, /* Reserved for JPEG extensions */
    M_JPG10 = 0xfa, /* Reserved for JPEG extensions */
    M_JPG11 = 0xfb, /* Reserved for JPEG extensions */
    M_JPG12 = 0xfc, /* Reserved for JPEG extensions */
    M_JPG13 = 0xfd, /* Reserved for JPEG extensions */
    M_COM = 0xfe /* Comment */
} JIF_MARKER;

/* Frame */

typedef enum {
    JIF_FRAME_MODE_ABBR_TABLE,          /* Abbreviated format for table-spec  (not a image) */
    JIF_FRAME_MODE_HIERARCHICAL,        /* Jif contains a DHP marker segment before non-differential frame or frames. */
    JIF_FRAME_MODE_NONE_HIERARCHICAL
} JIF_FRAME_MODE;

typedef struct  {
    unsigned int C:8;     /* Component identifier */
    unsigned int H:4;     /* 1 ~ 4, Horizontal sampling factor: (component horizontal dimension) / X */
    unsigned int V:4;     /* 1 ~ 4, Vertical sampling factor: (component vertical dimension) / Y */
    unsigned int Tq:8;    /* Quantization table destination selector */
} JIF_FRAME_COMPONENT;    /* pointer to array of SOF_COMP */

typedef uint16_t width_t;

typedef struct {
    unsigned int Lf;            /* Frame header length */
    unsigned int P;             /* Sample precision */
    width_t Y;             /* max Number of sample lines in source image */
    width_t X;             /* max Number of samples per line */
    unsigned int Nf;            /* Number of image components in frame ( 1 ~ 255 ) */
    JIF_FRAME_COMPONENT * comps;    /* ( in this frame ) */
    JIF_FRAME_MODE mode;
} JIF_FRAME;

/* Scan */
typedef struct {
    unsigned int Cs:8;    /* scan component selector */
    unsigned int Td:4;    /* DC entropy coding table selector */
    unsigned int Ta:4;    /* AC entropy coding table selector */
    coeff_t PRED;         /* codec helper: latest decoded DC */
} JIF_SCAN_COMPONENT;

typedef struct {
    unsigned int Ls:16;     /* length of Scan Header */
    unsigned int Ns:8;      /* Number of image components in scan */
    unsigned int Ss:8;      /* Start of spectral or predictor selection */
    unsigned int Se:8;      /* End of spectral selection */
    unsigned int Ah:4;      /* Successive approximation bit position high */
    unsigned int Al:4;      /* Successive approximation bit position low or point transform */
    JIF_SCAN_COMPONENT * comps;
} JIF_SCAN;

typedef struct jif_scanner {
    byte * pjif;
    jif_offset size;
    jif_offset b;   /* (in bits) scanner bit cursor */
    jif_offset m;   /* last marker */
    uint8_t    bit_cnt;     /* jhuff:: nextbit() helper */
    byte       bit_B;       /* jhuff:: nextbit() helper */
    byte       bit_nextbit; /* jhuff:: nextbit() helper */
} JIF_SCANNER;

JIF_SCANNER * jif_new_scanner(byte * jif_array, jif_offset array_size);
JIF_SCANNER * jif_copy_scanner(JIF_SCANNER *);
void jif_del_scanner(JIF_SCANNER * );

jif_offset jif_get_offset(JIF_SCANNER *);
bool jif_is_marker_byte(byte b);

bool jif_scan_next_marker(JIF_SCANNER * );
JIF_MARKER jif_prob_next_marker(JIF_SCANNER * s);   /* return marker or 0x00 */
JIF_MARKER jif_current_marker(JIF_SCANNER * s);
bool jif_scan_next_maker_of(JIF_MARKER e_marker, JIF_SCANNER * s );

byte jif_current_byte(JIF_SCANNER * s);
byte jif_next_byte(JIF_SCANNER * s);
byte jif_scan_next_byte(JIF_SCANNER * s);
uint16_t jif_scan_2_bytes(JIF_SCANNER * s);
uint32_t jif_scan_4_bytes(JIF_SCANNER * s);

uint16_t jif_scan_t_bits(JIF_SCANNER * s, jif_offset t);    /* haff code is less than 16 bits */
uint16_t jif_scan_next_bit(JIF_SCANNER * s);

#endif /* jif_h */
