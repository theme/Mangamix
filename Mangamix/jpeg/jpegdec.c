//
//  jpegdec.c
//  Mangamix
//
//  Created by theme on 28/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#include "jpegdec.h"


pinfo j_dec_new(void) {
    pinfo p = (pinfo) calloc(1, sizeof(struct J_DEC_INFO));
    p->img = jimg_new();
    p->bmp = jbmp_new();
    p->f_para.comps = malloc(0);  /* so that dec_read_sof() can freely realloc memory on it, and free. */
    p->scan_jth_params = malloc(0);
    
    for(int i =0; i < JTBL_NUM; i++){
        p->jH[i] = jif_huff_new();
    }
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

            dinfo->tQ[Tq].precison = ( 0 == Pq ) ? 8 : 16;
            while (offset < Lq) {
                for (int i = 0 ; i < 64 ; i++){
                    if ( 0 == Pq ){
                        uint8_t c = jif_scan_next_byte(s); offset++;
                        dinfo->tQ[Tq].coeff_a.Q8[i] = c;
                    } else /* 1 == Pq */ {
                        uint16_t c = jif_scan_2_bytes(s); offset += 2;
                        dinfo->tQ[Tq].coeff_a.Q16[i] = c;
                    }
                }
            }
            dinfo->is_dct_based = true;
        }
            break;
        case M_DHT:
            printf("%x @%llu DHT\n", m, jif_get_offset(s));
        {   uint16_t Lh = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            
            while(offset < Lh){
                byte b = jif_scan_next_byte(s); offset++;
                uint8_t Tc = ( b & 0xF0 )  >> 4 ;       /* table class 0: DC, 1: AC */
                uint8_t Th = b & 0x0F;
                dinfo->jH[Th]->Tc = Tc;
                
                for(int i = 0; i < 16; i++){
                    dinfo->jH[Th]->bits[i] = jif_scan_next_byte(s); offset++;
                }
                
                for(int i = 0; i < 16; i++){
                    for(int j = 0; j < dinfo->jH[Th]->bits[i]; j++){
                        jif_huff_set_val(dinfo->jH[Th], j, jif_scan_next_byte(s));
                        offset++;
                    }
                }
                
                /* gen huffman table */
                dinfo->tH[Th] = jtbl_huff_from_jif(dinfo->jH[Th]);
            }
            
        }
            break;
        case M_DAC:
            printf("%x @%llu DAC\n", m, jif_get_offset(s));
            /* Arithmetic coding conditioning */
            dinfo->is_use_arithmetic_coding = true;
            break;
        case M_DRI:
            printf("%x @%llu DRI\n", m, jif_get_offset(s));
        {   uint16_t Lr = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            if ( 4 != Lr ) {err("error DRI length\n");}
            uint16_t Ri = jif_scan_2_bytes(s);  offset+=2;
            dinfo->Ri = Ri;       /* restart interval is enabled if > 0 */
        }
            break;
        case M_COM:
            printf("%x @%llu COMMENT\n", m, jif_get_offset(s));
        {   uint16_t Lc = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            byte b;
            printf("COM:");
            while (offset < Lc){
                b = jif_scan_next_byte(s);  offset++;
                putchar(b);
            }
            printf("\n");
        }
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

/* private: read one sof marker. */
bool dec_read_sof(pinfo dinfo, JIF_SCANNER * s){
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
            dinfo->f_para.Lf = jif_scan_2_bytes(s);
            dinfo->f_para.P = jif_scan_next_byte(s);    /* sample precision of components in frame */
            dinfo->f_para.Y = jif_scan_2_bytes(s);
            dinfo->f_para.X = jif_scan_2_bytes(s);
            dinfo->f_para.Nf = jif_scan_next_byte(s);
            dinfo->f_para.comps = realloc(dinfo->f_para.comps, dinfo->f_para.Nf*sizeof(JIF_COMPONENT_PARAM));
            
            byte b, H, V;
            for(int i = 0; i < dinfo->f_para.Nf; i++){
                dinfo->f_para.comps[i].C = jif_scan_next_byte(s);
                b = jif_scan_next_byte(s);
                H = b >> 4;
                V = 0x0f & b;
                dinfo->f_para.comps[i].H = H;
                dinfo->f_para.comps[i].V = V;
                dinfo->f_para.comps[i].Tq = jif_scan_next_byte(s);
            }
            
            return true;
        }
        default:
            printf("%x @%llu NOT M_SOFx\n", m, jif_get_offset(s));
            break;
    }
    return false;
}


void dec_update_img_after_sof (pinfo dinfo){
    JIF_COMPONENT_PARAM * p;
    for(int i = 0; i < dinfo->f_para.Nf; i++){
        p = &dinfo->f_para.comps[i];
        jimg_set_components(dinfo->img, p->C, p->H, p->V, dinfo->f_para.P);
    }
    dinfo->img->X = dinfo->f_para.X;
    dinfo->img->Y = dinfo->f_para.Y;
}

void dec_update_bmp_after_sof (pinfo dinfo){
    
    JIMG_BITMAP b = *dinfo->bmp;
    
    if ( b.width < dinfo->f_para.X )
        b.width = dinfo->f_para.X;
    
    if ( b.height < dinfo->f_para.Y )
        b.height = dinfo->f_para.Y;
    
    dec_update_img_after_sof(dinfo);
    
    b.bits_per_component = dinfo->f_para.P;
    b.bits_per_pixel = dinfo->f_para.P * dinfo->img->num_of_components;
    
}

/* private: try read one sof marker. */
bool dec_prob_read_a_sof_param(pinfo dinfo, JIF_SCANNER * s){
    JIF_SCANNER * sof_s = jif_copy_scanner(s);  /* new */
    bool success = false;

    do {
        if(dec_read_sof(dinfo, sof_s)){
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
        
        /* handle different mode */
        switch (jif_get_current_marker(s_soi)) {
            case M_EOI:     /* Abbreviated format for table-specification data */
                continue;
                break;
            case M_DHP:     /* Hierarchical mode */
                continue;
                break;
            default:
                break;
        }
        
        /* read SOF  */
        if (dec_prob_read_a_sof_param(dinfo, s_soi)){   /* prob_read won't affect s_soi */
            success = true;
            break;
        }
        
        /* to get image size */
        dec_update_bmp_after_sof(dinfo);
    }
    
    jif_del_scanner(s_soi);
    return success;
}

unsigned long j_info_get_width(pinfo dinfo){
    return dinfo->img->X;
};

unsigned long j_info_get_height(pinfo dinfo){
    return dinfo->img->Y;
};

JIMG_COLOR_SPACE j_info_get_colorspace(pinfo dinfo){
    return dinfo->bmp->color_space;
}

int j_info_get_num_of_components(pinfo dinfo){
    return dinfo->img->num_of_components;
}
int j_info_get_component_depth(int comp_i, pinfo dinfo){
    return dinfo->bmp->bits_per_component;    /* TODO : "a decoder with appropriate accuracy" */
}

void * j_info_get_bmp_data(pinfo dinfo){
    return dinfo->bmp->data;
}
size_t j_info_get_bmp_data_size(pinfo dinfo){
    return dinfo->bmp->data_size;
}



bool dec_read_scan_header(pinfo dinfo, JIF_SCANNER * s){
    // TODO read sos, set up decoding MCU cache ( 4:2:1 calculate )
    JIF_MARKER m = jif_get_current_marker(s);
    switch (m) {
        case M_SOS:
            printf("%x @%llu SOS\n", m, jif_get_offset(s));
        {   /* scan header */
            uint16_t Ls = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            
            uint8_t Ns = jif_scan_next_byte(s); offset++;   /* number of image component in scan */
            dinfo->scan_param.Ns = Ns;
            dinfo->scan_jth_params = realloc(dinfo->scan_jth_params,
                                                    Ns * sizeof(JIF_SCAN_PARAM_jth));
            byte b;
            dinfo->sample_per_MCU = 0;
            int Cs;
            for(int j=0; j < Ns; j++){
                Cs =jif_scan_next_byte(s); offset++;   /* Scan component selector */
                dinfo->scan_jth_params[j].Cs = Cs;
                b = jif_scan_next_byte(s); offset++;
                dinfo->scan_jth_params[j].Td = ( b >> 4 );    /* Specifies one of four possible DC entropy coding table */
                dinfo->scan_jth_params[j].Ta = ( 0x0f & b );  /* Specifies one of four possible AC entropy coding table */
                
                dinfo->sample_per_MCU += dinfo->f_para.comps[Cs].H * dinfo->f_para.comps[Cs].V;
            }
            
            if( Ns > 1 ) {
                if( dinfo->sample_per_MCU > 10){
                    err("sample_per_MCU=sum_j_( H * V ) should < 10");
                    return false;
                }
            } else if ( 1 == Ns ){
                dinfo->sample_per_MCU = 1;
            }
            
        }
            return true;
        default:
            break;
    }
    return false;
}

void dec_reset_decoder(pinfo dinfo, JIF_SCANNER * s){
    if(dinfo->is_use_arithmetic_coding){
        /* dec_init() arithmetic decoder */
    }
    
    if(dinfo->is_dct_based){
        for(int j = 0; j< dinfo->scan_param.Ns; j++){
            /* TODO : set DC components' PREC -> 0 */
        }
    }
}

// Data unit (in en/decoder) :
//      A. 8x8 sample matrix ( DCT based process )
//      B. 1 sample ( lossless process )

void dec_decode_data_unit(pinfo dinfo, JIF_SCANNER * s){
    
    uint8_t ZZ[DCTSIZE];    /* TODO: MCU buffer, tgt image */
    
    /* decode DC coeff, using DC table specified in scan header. */
    ZZ[0] = PRED + DIFF;
    
    /* decode AC coeff, using AC table specified in scan header. */
    int size = 2 * DCTSIZE_ROOT;
    int i = 0,j = 0;
    int s = 0; // sum
    int c = 1;
    for( s = 1 ; s < size ; s++ ) {
        
        if( s % 2 == 0 ){
            // i decrease loop
            for ( i = s; 0 <= i ; i--){
                j = s - i;
                if ( i < DCTSIZE_ROOT && j < DCTSIZE_ROOT ){
                    //TODO ZZ(c++) =
                }
            }
        } else {
            // i increase loop
            for ( i = 0; i <= s; i++){
                j = s - i;
                if ( i < DCTSIZE_ROOT && j < DCTSIZE_ROOT ){
                    //TODO ZZ(c++) =
                }
            }
        }
    }
    
    
    /* dequantize using table destination specified in the frame header. */
    
    /* calculate the inverse 8 × 8 DCT. */
}

void dec_decode_MCU(pinfo dinfo, JIF_SCANNER * s){
    
    if(dinfo->is_dct_based){
        /* TODO : Nb: number of data unit in a MCU */
        int Nb = 0;
        
        if (dinfo->scan_param.Ns == 1){ /* non interleaved */
            Nb = 1;
        } else {    /* calculate from current scan & frame */
            for ( int i; i < dinfo->scan_param.Ns; i++){
                JIF_SCAN_PARAM_jth sp = dinfo->scan_jth_params[i];
                JIF_COMPONENT_PARAM cp = dinfo->f_para.comps[sp.Cs];
                Nb += cp.H * cp.V;
            }
        }
        
        /* decode */
        for (int N = 0; N < Nb; N++){
            dec_decode_data_unit(dinfo, s);
        }
    }
}

bool dec_read_DNL(pinfo dinfo, JIF_SCANNER *s ){
    uint16_t Ld = jif_scan_2_bytes(s);
    uint16_t offset = 2;
    if ( 4 != Ld ){
        err("error DNL\n");
        return false;
    }
    uint8_t NL = jif_scan_2_bytes(s); offset+=2;   /* number of image component in scan */
    if( 0 == dinfo->f_para.Y ){
        dinfo->f_para.Y = NL;
    } else {
        err("err: redefine max number of lines in image.\n");
        return false;
    }
    return true;
}

void dec_decode_restart_interval(pinfo dinfo, JIF_SCANNER * s, unsigned int Rm){
    
    if(dinfo->is_dct_based){
        /* TODO set the DC prediction (PRED) to zero for all components in the scan */
        /* Decoding model for DC coefficients
         The decoded difference, DIFF, is added to PRED, the DC value from the most recently decoded 8 × 8 block from the same component. Thus ZZ(0) = PRED + DIFF.
         At the beginning of the scan and at the beginning of each restart interval, the prediction for the DC coefficient is initialized to zero.
         */
        
        for(int j = 0; j < dinfo->scan_param.Ns; j++){
            int c = dinfo->scan_jth_params[j].Cs;
        }
    }
    
    for(int i=0; i < Rm; i++ ){
        dec_decode_MCU(dinfo, s);
    }
    
    /* test next marker : RST or DNL */
    JIF_MARKER m = jif_get_current_marker(s);
    
    if (M_DNL == m){
        dec_read_DNL(dinfo, s);
        return;
    }
    
    if ( M_RST0 <= m && m <= M_RST7) {
        /* optional error recovery : compare expected restart interval number to the one in the marker.
         * filling lost lines with some data.
         */
        
        return;
    }
}

unsigned int MCU_per_scan(pinfo dinfo){ /* number of MCU per scan */
    unsigned int H , V , Cs, x, y, r;
    
    if( dinfo->is_dct_based ) {
        r = 8;
    } else {
        r = 1;
    }
    /* for decoding, any component in the scan gives H and V (if encoding is normal) */
    Cs = dinfo->scan_jth_params[0].Cs;
    H = dinfo->f_para.comps[Cs].H;
    V = dinfo->f_para.comps[Cs].V;
    
    x = dinfo->f_para.X / H / r;
    y = dinfo->f_para.Y / V / r;
    
    return x * y;
}

/* note : a scan may enable restart interval Ri */
bool dec_decode_scan(pinfo dinfo, JIF_SCANNER * s){
    if(M_SOS != jif_get_current_marker(s)){
        return false;
    }
        
    if(!dec_read_scan_header(dinfo, s)){
        return false;
    }
    
    // How many intervals are there in this scan ?
    // number = total MCU per scan / MCU per interval (Ri)
    // A scan may choose some components
    unsigned int mps = MCU_per_scan(dinfo);
    unsigned int R = mps / dinfo->Ri;
    unsigned int Rr = mps % dinfo->Ri;
    
    for( int r = 0 ; r < R ; r++){
        dec_decode_restart_interval(dinfo, s, dinfo->Ri);
    }
    
    if ( 0 != Rr){
        dec_decode_restart_interval(dinfo, s, Rr);
    }
    
    return true;
}


bool dec_reserve_MCU_buffer(pinfo dinfo, JIF_SCANNER * s_soi){
    /* decide data unit number */
    
}

bool dec_reserve_img_buffer(pinfo dinfo, JIF_SCANNER *s_soi){
    dec_update_img_after_sof(dinfo);
}

bool j_dec_decode(pinfo dinfo){
    if( 0 == dinfo->src ) {
        return false;
    }
    
    bool success = false;
    
    JIF_SCANNER *s_soi = jif_new_scanner(dinfo->src, dinfo->src_size);
    
    /*  TODO: now only support baseline DCT image. */
    while( jif_scan_next_maker_of(M_SOI, s_soi) ){
        dinfo->Ri = 0;  /* SOI disable restart interval. Need a DRI to re-enable. */
        
        /* read tables | misc. */
        if (!dec_read_tables_misc(dinfo, s_soi)){
            err("> Error reading tables|misc." );
            continue;
        }
        
        /* read markers before SOS marked scans */
        switch (jif_get_current_marker(s_soi)) {
            case M_EOI:
                dinfo->f_mode = JIF_FRAME_MODE_ABBR_TABLE;
                err("> Do not support mode_abbr_table yet." );
                goto cleanup;
                break;
            case M_DHP:
                dinfo->f_mode = JIF_FRAME_MODE_HIERARCHICAL;
                err("> Do not support mode_hierarchical yet." );
                goto cleanup;
                break;
            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
                dinfo->f_mode = JIF_FRAME_MODE_NONE_HIERARCHICAL;
                if(dec_read_sof(dinfo, s_soi)){
                    if(!dec_read_tables_misc(dinfo, s_soi))
                        break;
                    
                    if(!dec_reserve_MCU_buffer(dinfo, s_soi))
                        break;
                    
                    if(!dec_reserve_img_buffer(dinfo, s_soi))
                        break;
                    
                    if(!dec_decode_scan(dinfo, s_soi))  /* first scan */
                        break;
                    
                    if( 0 == dinfo->f_para.Y ){ /* the number of line is defined by DNL after first scan. */
                        if(M_DNL == jif_get_current_marker(s_soi)){
                            dec_read_DNL(dinfo, s_soi);
                        }
                    }
                    
                    while(dec_decode_scan(dinfo, s_soi)){   /* 2nd scan and on */
                        
                    }
                    
                    success = true;
                }
                break;
            default:
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

void j_info_release_bmp_data(void *info, const void *data, size_t size){
    pinfo dinfo = (pinfo) info;
    jbmp_free(dinfo->bmp);
}

void j_dec_destroy(pinfo dinfo){
    jimg_free(dinfo->img);
    jbmp_free(dinfo->bmp);
    free(dinfo->f_para.comps);
    free(dinfo->scan_jth_params);
    free(dinfo);
}
