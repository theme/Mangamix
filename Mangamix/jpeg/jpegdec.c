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
    p->frame.comps = malloc(0);  /* so that dec_read_sof() can freely realloc memory on it, and free. */
    p->scan.comps = malloc(0);
    
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

JIF_FRAME_COMPONENT * frame_comp(pinfo dinfo, uint8_t Csj){
    for ( int j = 0 ; j < dinfo->frame.Nf; j++ ){
        if ( Csj == dinfo->frame.comps[j].C) {
            return &dinfo->frame.comps[j];
        }
    }
    return NULL;
}

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
                        dinfo->tQ[Tq].coeff_a[i] = c;
                    } else /* 1 == Pq */ {
                        uint16_t c = jif_scan_2_bytes(s); offset += 2;
                        dinfo->tQ[Tq].coeff_a[i] = c;
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
            dinfo->scan_Ri = jif_scan_2_bytes(s);  offset+=2; /* restart interval is enabled if > 0 */
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
            dinfo->frame.Lf = jif_scan_2_bytes(s);
            dinfo->frame.P = jif_scan_next_byte(s);    /* sample precision of components in frame */
            dinfo->frame.Y = jif_scan_2_bytes(s);
            dinfo->frame.X = jif_scan_2_bytes(s);
            dinfo->frame.Nf = jif_scan_next_byte(s);
            dinfo->frame.comps = realloc(dinfo->frame.comps, dinfo->frame.Nf * sizeof(JIF_FRAME_COMPONENT));
            
            byte b, H, V;
            for(int i = 0; i < dinfo->frame.Nf; i++){
                dinfo->frame.comps[i].C = jif_scan_next_byte(s);
                b = jif_scan_next_byte(s);
                H = b >> 4;
                V = 0x0f & b;
                dinfo->frame.comps[i].H = H;
                dinfo->frame.comps[i].V = V;
                dinfo->frame.comps[i].Tq = jif_scan_next_byte(s);
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
    JIF_FRAME_COMPONENT * p;
    for(int i = 0; i < dinfo->frame.Nf; i++){
        p = &dinfo->frame.comps[i];
        jimg_set_components(dinfo->img, p->C, p->H, p->V, dinfo->frame.P);
    }
    dinfo->img->X = dinfo->frame.X;
    dinfo->img->Y = dinfo->frame.Y;
}

void dec_update_bmp_after_sof (pinfo dinfo){
    
    JIMG_BITMAP b = *dinfo->bmp;
    
    if ( b.width < dinfo->frame.X )
        b.width = dinfo->frame.X;
    
    if ( b.height < dinfo->frame.Y )
        b.height = dinfo->frame.Y;
    
    dec_update_img_after_sof(dinfo);
    
    b.bits_per_component = dinfo->frame.P;
    b.bits_per_pixel = dinfo->frame.P * dinfo->img->num_of_components;
    
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
//            uint16_t Ls = jif_scan_2_bytes(s);
            jif_scan_2_bytes(s);
            uint16_t offset = 2;
            
            uint8_t Ns = jif_scan_next_byte(s); offset++;   /* number of image component in scan */
            dinfo->scan.Ns = Ns;
            dinfo->scan.comps = realloc(dinfo->scan.comps,
                                        Ns * sizeof(JIF_SCAN_COMPONENT));
            byte b;
            dinfo->du_per_MCU = 0;
            int Cs;
            for(int j=0; j < Ns; j++){
                Cs =jif_scan_next_byte(s); offset++;   /* Scan component selector */
                dinfo->scan.comps[j].Cs = Cs;
                b = jif_scan_next_byte(s); offset++;
                dinfo->scan.comps[j].Td = ( b >> 4 );    /* Specifies one of four possible DC entropy coding table */
                dinfo->scan.comps[j].Ta = ( 0x0f & b );  /* Specifies one of four possible AC entropy coding table */
                
                JIF_FRAME_COMPONENT * c = frame_comp(dinfo, Cs);
                dinfo->du_per_MCU += c->H * c->V;
            }
            
            if( Ns > 1 ) {
                if( dinfo->du_per_MCU > 10){
                    err("sample_per_MCU=sum_j_( H * V ) should < 10");
                    return false;
                }
            } else if ( 1 == Ns ){
                dinfo->du_per_MCU = 1;
            }
            
        }
            return true;
        default:
            break;
    }
    return false;
}

// Data unit :
//      A. 8x8 sample matrix ( DCT based process )
//      B. 1 sample ( lossless process )
unsigned int data_block_width(pinfo dinfo){
    return dinfo->is_dct_based ? 8: 1;
}

void dec_decode_data_unit(pinfo dinfo, JIF_SCANNER * s,
                          unsigned int sj, unsigned int du_x   , unsigned int du_y ){
    
    if ( dinfo->is_dct_based ){
        coeff_t ZZ[DCTSIZE];
        
        /* decode DC coeff, using DC table specified in scan header. */
        JIF_SCAN_COMPONENT * sp = &dinfo->scan.comps[sj];
        huff_size T = jhuff_decode(dinfo->tH[sp->Td], s);
        coeff_t DIFF = jhuff_receive(T, s);
        DIFF = jhuff_extend(DIFF, T);
        ZZ[0] = sp->PRED + DIFF;
        
        /* decode AC coeff, using AC table specified in scan header. */
        for (unsigned int K = 1; K != 63; K++){
            for( int i = 1; i < DCTSIZE; i++){
                ZZ[i] = 0;
            }
            
            huff_size RS = jhuff_decode(dinfo->tH[sp->Ta], s);
            coeff_t SSSS = RS % 16;
            coeff_t RRRR = RS >> 4;
            coeff_t R = RRRR;
            
            if ( 0 == SSSS ) {
                if ( 15 == R ) {    /* 0xF0: a run length of 15 zero coefficients followed by a coefficient of zero amplitude */
                    K += 15;
                    continue;
                } else {    /* EOB : all remaining coeff is 0 */
                    break;
                }
            } else {
                K += R;
                ZZ[K] = jhuff_receive(SSSS, s);
                ZZ[K] = jhuff_extend(ZZ[K], SSSS);
            }
        }
        
        /* dequantize using table destination specified in the frame header. */
        //Annex A
        JIF_FRAME_COMPONENT * cp = frame_comp(dinfo, dinfo->scan.comps[sj].Cs);
        jquant_dequant(&(dinfo->tQ[cp->Tq]), ZZ);
        
        /* calculate 8 × 8 inverse DCT. */
        //A.3.3
        double IDCT[DCTWIDTH][DCTWIDTH];
        j_idct_ZZ(IDCT, ZZ);
        
        /* level shift after IDCT */
        uint16_t y, x;
        for (y=0; y<DCTWIDTH; y++) {
            for (x=0; x<DCTWIDTH; x++) {
                IDCT[y][x] += 1 << (dinfo->frame.P - 1);
            }
        }
        
        /* write to img */
        for (y=0; y<DCTWIDTH; y++) {
            for (x=0; x<DCTWIDTH; x++) {
                jimg_write_sample(dinfo->img, sp->Cs, du_x + x, du_y + y, IDCT[y][x]);
            }
        }
    }
}

void dec_decode_MCU(pinfo dinfo, JIF_SCANNER * s){
    
    for ( int j = 0; j < dinfo->scan.Ns; j++){
        JIF_SCAN_COMPONENT sp = dinfo->scan.comps[j];
        JIF_FRAME_COMPONENT * cp = frame_comp(dinfo, sp.Cs);
        
        for (int h = 0; h < cp->H; h++){
            for (int v = 0; v < cp->V; v++){
                int du_x = dinfo->dec_MCU_i * cp->H + h;    /* data unit x */
                int du_y = dinfo->dec_MCU_i * cp->V + v;
                dec_decode_data_unit(dinfo, s, j, du_x, du_y);
            }
        }
    }
    
    dinfo->dec_MCU_i++;
}

bool dec_read_DNL(pinfo dinfo, JIF_SCANNER *s ){
    uint16_t Ld = jif_scan_2_bytes(s);
    uint16_t offset = 2;
    if ( 4 != Ld ){
        err("error DNL\n");
        return false;
    }
    uint8_t NL = jif_scan_2_bytes(s); offset+=2;   /* number of image component in scan */
    if( 0 == dinfo->frame.Y ){
        dinfo->frame.Y = NL;
    } else {
        err("err: redefine max number of lines in image.\n");
        return false;
    }
    return true;
}

void dec_decode_restart_interval(pinfo dinfo, JIF_SCANNER * s, unsigned int Rm){
    /* reset on restart */
    if(dinfo->is_dct_based){
        for(int j = 0; j < dinfo->scan.Ns; j++){
            dinfo->scan.comps[j].PRED = 0;
        }
    }
    
    for(int i=0; i < Rm; i++ ){
        dec_decode_MCU(dinfo, s);
    }
    
    /* test next marker : RST or DNL */
    JIF_MARKER m = jif_get_current_marker(s);
    
    if (M_DNL == m){
        printf("%x @%llu DNL\n", m, jif_get_offset(s));
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

/* note : a scan may enable restart interval Ri */
bool dec_decode_scan(pinfo dinfo, JIF_SCANNER * s){
    if(M_SOS != jif_get_current_marker(s)){
        return false;
    }
        
    if(!dec_read_scan_header(dinfo, s)){
        return false;
    }
    
    // How many intervals are there in this scan ?
    // expected number = total MCU per scan / MCU per interval (Ri)
    // A scan may choose some components
    
    unsigned int X, Y, r;
    
    r = data_block_width(dinfo);
    
    for(int j = 0; j < dinfo->scan.Ns; j++){
        JIF_SCAN_COMPONENT * sc =  &dinfo->scan.comps[j];
        JIF_FRAME_COMPONENT * fc = frame_comp(dinfo, sc->Cs);
        
        X = dinfo->frame.X / fc->H / r;    /* data units block */
        Y = dinfo->frame.Y / fc->V / r;
        
        dinfo->MCU_per_scan = X * Y;    /* same for each component in scan */
        
        dinfo->scan.comps[j].PRED = 0;
    }
    
    unsigned int R = dinfo->MCU_per_scan / dinfo->scan_Ri;
    unsigned int Rr = dinfo->MCU_per_scan % dinfo->scan_Ri;
    
    dinfo->dec_MCU_i = 0;
    
    for( int r = 0 ; r < R ; r++){
        dec_decode_restart_interval(dinfo, s, dinfo->scan_Ri);
    }
    
    if ( 0 != Rr){
        dec_decode_restart_interval(dinfo, s, Rr);
    }
    
    return true;
}

bool j_dec_decode(pinfo dinfo){
    if( 0 == dinfo->src ) {
        return false;
    }
    
    bool success = false;
    
    JIF_SCANNER *s_soi = jif_new_scanner(dinfo->src, dinfo->src_size);
    
    /*  TODO: now only support baseline DCT image. */
    while( jif_scan_next_maker_of(M_SOI, s_soi) ){
        dinfo->scan_Ri = 0;  /* SOI disable restart interval. Need a DRI to re-enable. */
        
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
                    
                    /* dec_reserve_img_buffer */
                    dec_update_img_after_sof(dinfo);
                    
                    if(!dec_decode_scan(dinfo, s_soi))  /* first scan */
                        break;
                    
                    if( 0 == dinfo->frame.Y ){ /* the number of line is defined by DNL after first scan. */
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
    free(dinfo->frame.comps);
    free(dinfo->scan.comps);
    free(dinfo);
}
