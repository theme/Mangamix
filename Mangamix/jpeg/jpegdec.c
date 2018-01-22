//
//  jpegdec.c
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#include "jpegdec.h"

/* inside decoder scope */
#include "jif.h"
#include "jframe.h"

#define err(S) fprintf(stderr, S)

/* DCT tables */
#define DCTSIZE     64

typedef union QUANT_TABLE_ARRAY {
    uint8_t     Q8[DCTSIZE];
    uint16_t    Q16[DCTSIZE];
} qtbl_arr;
typedef struct QUANT_TABLE {
    uint8_t     precison;   /* 8 | 16 */
    qtbl_arr    coeff_a;      /* DCT coeff array in zig-zag order */
} J_QTABLE;

typedef enum {
    J_DEC_NEW,
    J_DEC_SET_SRC,
    J_DEC_HEADER,
    J_DEC_FRAMES
} J_DEC_STAT;

typedef struct {
    uint16_t    width;    /* jpeg usual size : 1 ~ 65535 */
    uint16_t    height;
    uint8_t     num_of_components;
    J_COLOR_SPACE   color_space;
    uint8_t     bits_per_pixel;
    uint8_t     bits_per_component;    /* TODO ? In decoded raw image, each component has same depth. */
} J_IMAGE;

typedef struct J_DEC_INFO {
    byte*           src;  /* source jif array (to be decoded) */
    jif_offset      src_size;
    JIF_SCANNER *   jif_scanner;
    bool            is_mode_hierarchical; /* Jif contains a DHP marker segment before non-differential frame or frames. */
    J_QTABLE        qtables[4];
    J_DEC_STAT      stat;
    J_ERR           err;
    J_IMAGE         img;
} * pinfo;

pinfo j_dec_new(void) {
    pinfo p = (pinfo) malloc(sizeof(struct J_DEC_INFO));
    p->stat = J_DEC_NEW;
    p->img.width = 0;
    p->img.height = 0;
    return p;
};

bool j_dec_set_src_array(unsigned char *src, unsigned long long size, pinfo dinfo) {
    dinfo->src = src;
    dinfo->src_size = size;
    dinfo->stat = J_DEC_SET_SRC;
    return true;
};

/* scan to jif header, read image meta data into dinfo */
bool j_dec_read_header(pinfo dinfo){
    JIF_MARKER m;
    J_FRAME f;
    
    dinfo->stat = J_DEC_HEADER;
    
    if( 0 == dinfo->src ) {
        return false;
    }
    JIF_SCANNER * s = jif_new_scanner(dinfo->src, dinfo->src_size);
    dinfo->jif_scanner = s;
    
    /* scan to SOI: start of image */
    if( !jif_scan_next_maker_of(M_SOI, s) ){
        return false;
    }
    
    /* TODO: set up decoder */
    
    
    while ( jif_scan_next_marker(s) ){
        m = jif_get_current_marker(s);
        switch (m) {
            /* interpret Table spec and misc (APP, COMMENT ... ) */
            case M_DHT:
                
                break;
            case M_DAC:
                
                break;
            case M_DQT:
            {   uint16_t Lq = (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
                uint16_t offset = 2;
                
                byte b = jif_scan_next_byte(s);
                offset++;
                uint8_t Pq = ( b & 0xF0 )  >> 4 ;
                uint8_t Tq = b & 0x0F;
                
                dinfo->qtables[Tq].precison = ( 0 == Pq ) ? 8 : 16;
                while (offset < Lq) {
                    for (int i = 0 ; i < 64 ; i++){
                        if ( 0 == Pq ){
                            uint8_t c = jif_scan_next_byte(s);
                            offset++;
                            dinfo->qtables[Tq].coeff_a.Q8[i] = c;
                        } else /* 1 == Pq */ {
                            uint16_t c = (jif_scan_next_byte(s) << 8) + jif_scan_next_byte(s);
                            offset += 2;
                            dinfo->qtables[Tq].coeff_a.Q16[i] = c;
                        }
                    }
                }
                break;
                
            }
            case M_DRI:
                
                break;
            case M_APP0:    /* JFIF versions >= 1.02 : embed a thumbnail image in 3 different formats. */
                
                break;
                
            case M_APP1:    /* Exif Attribute Information | XMP */
                
                break;
            case M_APP2:    /* ICC */
                
                break;
            case M_APP13:    /* Photoshop layers, path, IPTC ... */
                
                break;
            case M_COM:
                
                break;
            case M_DHP:
                dinfo->is_mode_hierarchical = true;
                return false; /* TODO */
                break;
            
            /* scan to a frame segment, and get image size */
            /* In case of hierarchical mode, there are multiple frames, scan all and take the biggest. */
            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
                jframe_read_jif(&f, s);
                dinfo->img.width = f.X;
                dinfo->img.height = f.Y;
                switch (f.Nf) {
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
                dinfo->img.bits_per_component = f.P; // TODO ?
                dinfo->img.bits_per_pixel = dinfo->img.bits_per_component * dinfo->img.num_of_components;
                return true;    /* got a image width and height */
            default:
                break;
        }
    }

    return false;   /* should have read a sof param and returned. */
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
    return dinfo->img.bits_per_component;    /* TODO : "a decoder with appropriate accuracy" */
}

bool j_dec_decode_frames(pinfo dinfo){
    dinfo->stat = J_DEC_FRAMES;
    
    return false;
}

bool j_dec_is_success(pinfo dinfo);

J_ERR j_info_get_error(pinfo dinfo){
    return dinfo->err;
}

void j_dec_destroy(pinfo dinfo){
    free(dinfo);
}
