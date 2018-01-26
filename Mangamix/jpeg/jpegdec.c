//
//  jpegdec.c
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#include "jpegdec.h"


pinfo j_dec_new(void) {
    pinfo p = (pinfo) malloc(sizeof(struct J_DEC_INFO));
    p->img.width = 0;
    p->img.height = 0;
    p->img.data = NULL;
    p->img.data_size = 0;
    return p;
};

bool j_dec_set_src_array(unsigned char *src, unsigned long long size, pinfo dinfo) {
    dinfo->src = src;
    dinfo->src_size = size;
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

            byte b = jif_scan_next_byte(s); offset++;
            uint8_t Pq = ( b & 0xF0 )  >> 4 ;
            uint8_t Tq = b & 0x0F;

            dinfo->qtables[Tq].precison = ( 0 == Pq ) ? 8 : 16;
            while (offset < Lq) {
                for (int i = 0 ; i < 64 ; i++){
                    if ( 0 == Pq ){
                        uint8_t c = jif_scan_next_byte(s); offset++;
                        dinfo->qtables[Tq].coeff_a.Q8[i] = c;
                    } else /* 1 == Pq */ {
                        uint16_t c = jif_scan_2_bytes(s); offset += 2;
                        dinfo->qtables[Tq].coeff_a.Q16[i] = c;
                    }
                }
            }
        }
            break;
        case M_DHT:
            printf("%x @%llu DHT\n", m, jif_get_offset(s));
        {   uint16_t Lh = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            
            while(offset < Lh){
                byte b = jif_scan_next_byte(s); offset++;
                uint8_t Tc = ( b & 0xF0 )  >> 4 ;
                uint8_t Th = b & 0x0F;
                dinfo->htables[Th].Tc = Tc;
                
                for(int i = 0; i < 16; i++){
                    dinfo->htables[Th].nL[i] = jif_scan_next_byte(s); offset++;
                }
                for(int i = 0; i < 16; i++){
                    uint8_t * a = malloc(sizeof(uint8_t) * dinfo->htables[Th].nL[i]);
                    dinfo->htables[Th].vArrays[i] = a;
                    for(int j = 0; j < dinfo->htables[Th].nL[i]; j++){
                         a[j] = jif_scan_next_byte(s); offset++;
                    }
                }
            }
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
            goto skipapp;
            break;

        case M_APP1:    /* Exif Attribute Information | XMP */
            printf("%x @%llu APP1 - Exif | XMP\n", m, jif_get_offset(s));
            goto skipapp;
            break;
        case M_APP2:    /* ICC */
            printf("%x @%llu APP2 - ICC\n", m, jif_get_offset(s));
            goto skipapp;
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
            goto skipapp;
            break;
        case M_APP13:    /* Photoshop layers, path, IPTC ... */
            printf("%x @%llu APP13 (PS?)\n", m, jif_get_offset(s));
            goto skipapp;
            break;
        case M_APP14:
            printf("%x @%llu APP14\n", m, jif_get_offset(s));
            goto skipapp;
            break;
        case M_APP15:
            printf("%x @%llu APP15\n", m, jif_get_offset(s));
        skipapp:
        {
            uint16_t Lp = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            /* TODO : (now skipping..) */
            while(offset < Lp){
                jif_scan_next_byte(s); offset++;
            }
            printf("> skipped to offset @%llu\n", jif_get_offset(s));
        }
            break;
        default:
            printf("%x @%llu NOT a table | misc marker.\n", m, jif_get_offset(s));
            return false;       /* NOT a tables | misc marker. */
    } /* end switch */

    return true;                /* IS a tables | misc marker. */
}

/* private: try read one sof marker. */
bool dec_read_tables_misc(pinfo dinfo, JIF_SCANNER * s){
    bool success = false;
    
    while( jif_scan_next_marker(s) ){
        if(dec_read_a_tbl_misc(dinfo, s)){
            success = true;     /* there exists >= 1 table | misc. */
        } else {
            break;  /* untile a marker NOT a table | misc. */
        }
    }
    return success;
}

bool dec_read_a_scan( pinfo dinfo, JIF_SCANNER * s ){
    return false;    // TODO
}

/* private: read one sof marker. */
bool dec_read_sof(pinfo dinfo, JIF_SCANNER * s, bool header_only){
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
            JIF_FRAME f;
            int c;
            byte b;
            f.Lf = jif_scan_2_bytes(s);
            f.P = jif_scan_next_byte(s);
            f.Y = jif_scan_2_bytes(s);
            f.X = jif_scan_2_bytes(s);
            f.Nf = jif_scan_next_byte(s);
            for(c = 0; c < f.Nf; c++){
                f.comps[c].C = jif_scan_next_byte(s);
                b = jif_scan_next_byte(s);
                f.comps[c].H = ( b >> 4 );
                f.comps[c].V = ( 0x0f & b );
                f.comps[c].Tq = jif_scan_next_byte(s);
            }
            
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
            /* calculate image size. e.g. 4:2:0 */  // TODO
            dinfo->img.bits_per_component = f.P;
            dinfo->img.bits_per_pixel = dinfo->img.bits_per_component * dinfo->img.num_of_components;
            
            if (header_only){
                return true;    /* got a image width and height */
            }
            
            /* multi scan loop */
            /* read tables | misc. (inside SOF) */
            while( dec_read_tables_misc(dinfo, s) ){
            }
            
            /* SOS (~ DNL) */
            
            while( dec_read_a_scan(dinfo, s)){
                
            }
            return true;
        }
        default:
            printf("%x @%llu NOT M_SOFx\n", m, jif_get_offset(s));
            break;
    }
    return false;
}

/* private: try read one sof marker. */
bool dec_prob_read_a_sof_param(pinfo dinfo, JIF_SCANNER * s){
    JIF_SCANNER * sof_s = jif_copy_scanner(s);  /* new */
    bool success = false;

    do {
        if(dec_read_sof(dinfo, sof_s, true)){
            success = true;
            break;
        }
    } while(jif_scan_next_marker(sof_s));

    jif_del_scanner(sof_s);                     /* del */
    return success;
}

/* scan jif file to get image size, color. */
bool j_dec_read_header(pinfo dinfo){
    if( 0 == dinfo->src ) {
        return false;
    }
    
    bool success = false;
    
    /* process every SOI segment in jif file */
    JIF_SCANNER *s_soi = jif_new_scanner(dinfo->src, dinfo->src_size);
    while( jif_scan_next_maker_of(M_SOI, s_soi) ){
        
        /* read tables | misc. */
        if (!dec_read_tables_misc(dinfo, s_soi)){
            err("> Error reading tables|misc." );
            continue;
        }
        
        switch (jif_get_current_marker(s_soi)) {
            case M_EOI:
                continue;
                break;
            case M_DHP:
                if (!dec_read_tables_misc(dinfo, s_soi)){
                    err("> Error reading tables|misc." );
                    continue;
                }
                break;
            case M_EXP:
                jif_scan_next_marker(s_soi);
                break;
            default:
                break;
        }
        
        /* read SOF to get image size */
        if (dec_prob_read_a_sof_param(dinfo, s_soi)){   /* prob_read won't affect s_soi */
            success = true;
            break;
        }
    }
    
    jif_del_scanner(s_soi);
    return success;
}

unsigned long j_info_get_width(pinfo dinfo){
    return dinfo->img.width;
};

unsigned long j_info_get_height(pinfo dinfo){
    return dinfo->img.height;
};

JIMG_COLOR_SPACE j_info_get_colorspace(pinfo dinfo){
    return dinfo->img.color_space;
}

int j_info_get_num_of_components(pinfo dinfo){
    return dinfo->img.num_of_components;
}
int j_info_get_component_depth(int comp_i, pinfo dinfo){
    return dinfo->img.bits_per_component;    /* TODO : "a decoder with appropriate accuracy" */
}

void * j_info_get_img_data(pinfo dinfo){
    return dinfo->img.data;
}
size_t j_info_get_img_data_size(pinfo dinfo){
    return dinfo->img.data_size;
}

void j_info_release_img_data(void *info, const void *data, size_t size){
    /* (*data, size) ignored, 'cause info has them. */
    pinfo dinfo = (pinfo) info;
    if(dinfo->img.data != NULL){
        free(dinfo->img.data);
        dinfo->img.data = NULL;
    }
}

bool j_dec_decode(pinfo dinfo){
    if( 0 == dinfo->src ) {
        return false;
    }
    
    bool success = false;
    
    /* process every SOI segment in jif file */
    JIF_SCANNER *s_soi = jif_new_scanner(dinfo->src, dinfo->src_size);
    while( jif_scan_next_maker_of(M_SOI, s_soi) ){
        
        /* read tables | misc. */
        if (!dec_read_tables_misc(dinfo, s_soi)){   /* TODO: should test is not thumbnail */
            err("> Error reading tables|misc." );
            continue;
        }
        
        switch (jif_get_current_marker(s_soi)) {
            case M_EOI:
                dinfo->f_mode = J_MODE_ABBR_TABLE;
                err("> Do not support mode_abbr_table yet." );
                goto cleanup;
                break;
            case M_DHP:
                dinfo->f_mode = J_MODE_HIERARCHICAL;
                err("> Do not support mode_hierarchical yet." );
                goto cleanup;
                break;
            default:
                dinfo->f_mode = J_MODE_NONE_HIERARCHICAL;
                break;
        }
        
        /* read SOF : a image */
        if (dec_read_sof(dinfo, s_soi, false)){
            success = true;
            break;
        }
    }
cleanup:
    jif_del_scanner(s_soi);
    return success;
}

J_ERR j_info_get_error(pinfo dinfo){
    return dinfo->err;
}

void j_dec_destroy(pinfo dinfo){
    j_info_release_img_data(dinfo, dinfo->img.data, dinfo->img.data_size);
    free(dinfo);
}
