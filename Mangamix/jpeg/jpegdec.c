//
//  jpegdec.c
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#include "jpegdec.h"

#define err(S) fprintf(stderr, S)

typedef enum {
    ERR_MALLOC,  /* not enough memory */
    ERR_UNKNOWN
} J_ERR;

typedef struct {
    byte r;
    byte g;
    byte b;
    byte a;
} J_PIXEL_RGBA;

typedef enum {
    J_DEC_NEW,
    J_DEC_SET_SRC,
    J_DEC_HEADER
} J_DEC_STAT;

typedef struct J_DEC_INFO {
    J_DEC_STAT stat;
    byte* src_arr;  /* source jif array (to be decoded) */
    jif_offset src_size;
    J_IMAGE img;
} *pinfo;


pinfo j_dec_new(void) {
    pinfo p = (pinfo) malloc(sizeof(struct J_DEC_INFO));
    p->stat = J_DEC_NEW;
    p->img.width = 0;
    p->img.height = 0;
    return p;
};

bool j_dec_set_src_array(byte *src, jif_offset size, pinfo dinfo){
    dinfo->src_arr = src;
    dinfo->src_size = size;
    dinfo->stat = J_DEC_SET_SRC;
    return true;
};

/* scan to jif header, read image meta data into dinfo */
bool j_dec_read_header(pinfo dinfo){
    if( 0 == dinfo->src_arr ) {
        return false;
    }
    JIF_SCANNER * scanner = jif_new_scanner(dinfo->src_arr, dinfo->src_size);
    JIF_SCANNER * eoi_scanner;
    jif_offset o, e;
    
    /* scan whole file until we got header */
    o = jif_scan_next_maker_of(M_SOI, scanner);
    if(!jif_is_marker(scanner)){
        err("no SOI");
        return false;  /* no SOI found */
    }
    
    eoi_scanner = jif_copy_scanner(scanner);
    e = jif_scan_next_maker_of(M_EOI, eoi_scanner);
    if(!jif_is_marker(eoi_scanner)){
        return false;  /* no EOI found */
    }
    
    /* process header */
    
    return true;
};

unsigned long j_info_get_width(pinfo dinfo){
    return dinfo->img.width;
};

unsigned long j_info_get_height(pinfo dinfo){
    return dinfo->img.height;
    
};
J_COLOR_SPACE j_info_get_colorspace(pinfo dinfo);
int j_info_get_num_of_components(pinfo dinfo);
int j_info_get_component_depth(int comp_i, pinfo dinfo);    /* depth of i th component */

bool j_dec_decode(pinfo dinfo);
bool j_dec_is_success(pinfo dinfo);
j_err j_info_get_error(pinfo dinfo);
j_pixel_rgba * j_dec_get_image_rgba(pinfo dinfo);   /* pointer to pixel array */
void j_dec_destroy(pinfo dinfo);
