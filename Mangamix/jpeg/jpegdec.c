//
//  jpegdec.c
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#include "jpegdec.h"
#include "jif.h"    /* inside decoder scope */

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
    bool is_mode_hierarchical; /* Jif contains a DHP marker segment before non-differential frame or frames. */
} *pinfo;


pinfo j_dec_new(void) {
    pinfo p = (pinfo) malloc(sizeof(struct J_DEC_INFO));
    p->stat = J_DEC_NEW;
    p->img.width = 0;
    p->img.height = 0;
    return p;
};

bool j_dec_set_src_array(unsigned char *src, unsigned long long size, pinfo dinfo) {
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
    JIF_SCANNER * frame_scanner;
    
    /* scan to SOI: start of image */
    if( !jif_scan_next_maker_of(M_SOI, scanner) ){
        return false;
    }
    
    /* scan to a frame segment, and get image size */
    /* In case of hierarchical mode, there are multiple frames, scan all and take the biggest. */
    frame_scanner = jif_copy_scanner(scanner);
    while( jif_scan_next_marker(frame_scanner)) {
        JIF_MARKER m = jif_get_current_marker(frame_scanner);
        switch (m) {
            case M_DHP:
                dinfo->is_mode_hierarchical = true;
                return false; /* TODO */
                break;
            case M_SOF0:    /* the only supported base line DCT (non-differential, Huffman coding) */
                jif_read_frame_param(frame_scanner);
                dinfo->img.width = frame_scanner->frame.X;
                dinfo->img.height = frame_scanner->frame.Y;
                switch (frame_scanner->frame.Nf) {
                    case 1:
                        dinfo->img.color_space = J_COLOR_GRAY;
                        dinfo->img.num_of_components = 1;
                        break;
                    case 3:
                        dinfo->img.color_space = J_COLOR_RGB;   /* output RGB */
                        dinfo->img.num_of_components = 3;
                        break;
                    case 4:
                        dinfo->img.color_space = J_COLOR_CMYK;
                        dinfo->img.num_of_components = 4;
                        break;
                    default:
                        break;
                }
                dinfo->img.bits_per_component = frame_scanner->frame.P; // TODO ?
                dinfo->img.bits_per_pixel = dinfo->img.bits_per_component * dinfo->img.num_of_components;
                return true;
            default:
                break;
        }
    }
    
    return false;
};

unsigned long j_info_get_width(pinfo dinfo){
    return dinfo->img.width;
};

unsigned long j_info_get_height(pinfo dinfo){
    return dinfo->img.height;
};

J_COLOR_SPACE j_info_get_colorspace(pinfo dinfo){
    return dinfo->img.color_space;
}

int j_info_get_num_of_components(pinfo dinfo){
    return dinfo->img.num_of_components;
}
int j_info_get_component_depth(int comp_i, pinfo dinfo){
    return dinfo->img.bits_per_component;    /* TODO */
}

bool j_dec_decode(pinfo dinfo);
bool j_dec_is_success(pinfo dinfo);
j_err j_info_get_error(pinfo dinfo);
j_pixel_rgba * j_dec_get_image_rgba(pinfo dinfo);   /* pointer to pixel array */
void j_dec_destroy(pinfo dinfo){
    free(dinfo);
}
