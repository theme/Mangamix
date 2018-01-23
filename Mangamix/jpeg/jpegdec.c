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
#define logger(S) printf(S)

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

/* Huffman tables */
typedef struct {
    byte    BITS[16];
    uint8_t     HUFFVAL[16];
} J_HUFF_TBL;

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

typedef struct J_DEC_INFO { /* decoder status (of a image segment between SOI and EOI markers */
    byte           *src;                    /* source jif array (to be decoded) */
    jif_offset      src_size;
    bool            is_mode_hierarchical;   /* Jif contains a DHP marker segment before non-differential frame or frames. */
    J_QTABLE        qtables[4];
    J_DEC_STAT      stat;
    J_ERR           err;
    J_IMAGE         img;
    /* JIF_SCANNER    *jif_scanner; */   /* needed? TO BE OR NOT TO BE, that is the question.
                                     How many images are there in a  jpeg/jif file ?
                                     When is scanner needed ?
                                     */
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

/* private: read top level tables|misc. */
bool dec_read_a_tbl_misc(pinfo dinfo, JIF_SCANNER *s){
    JIF_MARKER m = jif_get_current_marker(s);
    printf("%x @%llu\n", m, jif_get_offset(s));
    
    switch (m) {
        case M_DQT:
            printf("%x @%llu DQT\n", m, jif_get_offset(s));
        {   uint16_t Lq = jif_scan_2_bytes(s);
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
                        uint16_t c = jif_scan_2_bytes(s);
                        offset += 2;
                        dinfo->qtables[Tq].coeff_a.Q16[i] = c;
                    }
                }
            }
        }
            break;
        case M_DHT:
            printf("%x @%llu DHT\n", m, jif_get_offset(s));
        {   uint16_t Lh = jif_scan_2_bytes(s); uint16_t offset = 2;
            
            byte b = jif_scan_next_byte(s); offset++;
        }
            break;
        case M_DAC:
            printf("%x @%llu DAC\n", m, jif_get_offset(s));
            
            break;
        case M_DRI:
            printf("%x @%llu DRI\n", m, jif_get_offset(s));
            
            break;
        case M_COM:
            printf("%x @%llu COMMENT\n", m, jif_get_offset(s));
            
            break;
        case M_APP0:    /* JFIF versions >= 1.02 : embed a thumbnail image in 3 different formats. */
            printf("%x @%llu APP0 - JFIF\n", m, jif_get_offset(s));
            
            break;
            
        case M_APP1:    /* Exif Attribute Information | XMP */
            printf("%x @%llu APP1 - Exif | XMP\n", m, jif_get_offset(s));
        {   /* try Exif */
            uint16_t filed_len = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            if ('E' == jif_scan_next_byte(s)
                && 'x' == jif_scan_next_byte(s)
                && 'i' == jif_scan_next_byte(s)
                && 'f' == jif_scan_next_byte(s)
                && 0x00 == jif_scan_next_byte(s)
                && 0x00 == jif_scan_next_byte(s) ){
                offset += 6;
                logger("> Exif\n");
                
                uint16_t byte_order = jif_scan_2_bytes(s); offset += 2;
                if( 0x4949 == byte_order ) {    /* little endian */
                    logger("> \x49\x49 little ending\n");
                } else if ( 0x4d4d == byte_order ) {    /* big endian */
                    logger("> \x4d\x4d big ending\n");
                }
                uint16_t byte_42 = jif_scan_2_bytes(s); offset += 2;
                if ( 0x002A != byte_42 ) {
                    printf(">   error reading Exif header at 0x0042.%xx\n", byte_42);
                }
                uint32_t IFD_offset =  jif_scan_4_bytes(s); offset += 4;
                printf("> IFD offset %d\n", IFD_offset);
                for(int i = 0 ; i < IFD_offset - 8; i++){   /* scan to IFD */
                    jif_scan_next_byte(s); offset++;
                }
                
                break;
                
                while(offset < filed_len){      /* read IFD */
                    uint16_t num_fields = jif_scan_2_bytes(s); offset +=2;
                    uint16_t tag = jif_scan_2_bytes(s); offset +=2;     /* IFD0 and IFD1 tag are the same as in TIFF */
                    uint16_t type = jif_scan_2_bytes(s); offset +=2;
                    uint32_t count = jif_scan_4_bytes(s); offset +=4;
                    uint32_t voffset = jif_scan_4_bytes(s); offset +=4;
                    uint32_t next_IFD_offset = jif_scan_4_bytes(s); offset +=4;
                    switch (tag) {
                        case 34665:
                            /* Exif IFD Pointer */
                            break;
                        case 34853:
                            /* GPS Info IFD Pointer */
                            break;
                        case 40965:
                            /* Interoperability IFD Pointer */
                            break;
                            
                        default:
                            break;
                    }
                    /* TODO - now skip through IFD contents */
                    for(int i = 0 ; i < next_IFD_offset && offset < filed_len; i++){
                        jif_scan_next_byte(s); offset++;
                    }
                }
            }
            
        }
            break;
        case M_APP2:    /* ICC */
            printf("%x @%llu APP2 - ICC\n", m, jif_get_offset(s));
            break;
        case M_APP3:
        case M_APP4:
        case M_APP5:
        case M_APP6:
        case M_APP7:
        case M_APP8:
        case M_APP9:
        case M_APP10:
        case M_APP11:
        case M_APP12:
            printf("%x @%llu APP[3,12]\n", m, jif_get_offset(s));
            break;
        case M_APP13:    /* Photoshop layers, path, IPTC ... */
            printf("%x @%llu APP13 (PS?)\n", m, jif_get_offset(s));
            break;
        default:
            printf("%x @%llu NOT a table | misc marker.\n", m, jif_get_offset(s));
            return false;       /* NOT a tables | misc marker. */
    } /* end switch */
    
    return true;                /* IS a tables | misc marker. */
}

/* private: try read one sof marker. */
bool dec_read_sof_param(pinfo dinfo, JIF_SCANNER * s){
    JIF_MARKER m = jif_get_current_marker(s);
    switch (m) {
        case M_SOF0:
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
            printf("%x @%llu SOF%d\n", m, jif_get_offset(s), m-M_SOF0);
        {
            J_FRAME f;
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
        }
        default:
            printf("%x @%llu NOT M_SOFx\n", m, jif_get_offset(s));
            break;
    }
    return false;
}

/* private: try read one sof marker. */
bool dec_read_all_current_tables_misc(pinfo dinfo, JIF_SCANNER * s){
    bool success = false;
    
    while( jif_scan_next_marker(s) ){
        if(dec_read_a_tbl_misc(dinfo, s)){
            success = true;
        } else {
            break;
        }
    }
    
    return success;
}

/* private: try read one sof marker. */
bool dec_prob_read_a_sof_param(pinfo dinfo, JIF_SCANNER * s){
    JIF_SCANNER * sof_s = jif_copy_scanner(s);  /* new */
    bool success = false;
    
    while( jif_scan_next_marker(sof_s) ){
        if(dec_read_sof_param(dinfo, sof_s)){
            success = true;
            break;
        }
    }
    
    jif_del_scanner(sof_s);                     /* del */
    return success;
}

/* private: read a scan in frame. ( possible more than one scan in a frame) */
bool dec_scans(pinfo dinfo, JIF_SCANNER * s) {
    JIF_MARKER m;
    
    while ( jif_scan_next_marker(s) ){
        m = jif_get_current_marker(s);
        printf("%x << dec_scans()\n", m);
        /* TODO */
        
        if ( M_EOI == m) {
            return true;
        }
    }
    
    return true;   /* should have read a sof param and returned. */
};

/* scan to jif header, read image meta data into dinfo */
bool j_dec_read_header(pinfo dinfo){
    if( 0 == dinfo->src ) {
        return false;
    }
    
    JIF_SCANNER *s = jif_new_scanner(dinfo->src, dinfo->src_size);
    
    if( !jif_scan_next_maker_of(M_SOI, s) ){
        err("> Error no image (SOI marker).");
        goto ertn;
    }
    
    dinfo->stat = J_DEC_HEADER; /* status : after SOI, reading header */
    
    if (!dec_read_all_current_tables_misc(dinfo, s)){
        err("> Error reading tables|misc." );
        goto ertn;
    }
    
    /* here scanner pointing to marker AFTER some continuous tables|misc. */
    
    if (!dec_prob_read_a_sof_param(dinfo, s)){  /* to get image size, color ... */
        goto ertn;
    }
    
    jif_del_scanner(s);
    return true;
    
ertn:
    jif_del_scanner(s);
    return false;
}

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
