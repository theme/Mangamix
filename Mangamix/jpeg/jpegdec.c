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
    p->img = 0;
    p->frame.comps = malloc(0);  /* so that dec_read_sof() can freely realloc memory on it, and free. */
    p->scan.comps = malloc(0);
    
    for(int i =0; i < JTBL_NUM; i++){
        p->tH[0][i] = jhuff_new();
        p->tH[1][i] = jhuff_new();
    }
    
    // init helper array for DCT algorithm
    int i, j;
    for (i=0; i<DCTWIDTH; i++) {
        for (j=0; j<DCTWIDTH; j++) {
            p->cos_array[i][j] = cos((2*i + 1) * j * M_PI / 16);
        }
    }
    
    return p;
};


void j_dec_reset(pinfo p){
    if (p){
        // free
        if ( 0 != p->img) {
            jimg_free(p->img);
        }
        free(p->frame.comps);
        free(p->scan.comps);
        
        
        // reset
        p->img = 0;
        p->frame.comps = malloc(0);
        p->scan.comps = malloc(0);

    }
}

void j_dec_free(pinfo dinfo){
    if(dinfo){
        jimg_free(dinfo->img);
        free(dinfo->frame.comps);
        free(dinfo->scan.comps);
        free(dinfo);
    }
}

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
    return 0;
}

/* This is definition from standard:
 * Components that do not satisfy this will construct wrong image.
 * ceil(X * H / Hmax)
 */
uint16_t comp_x(pinfo dinfo, uint16_t frame_x, uint8_t comp_id){
    
    JIF_FRAME_COMPONENT * p = frame_comp(dinfo, comp_id);
    
    if(p){
        return ceil( (double)frame_x * p->H/ dinfo->frame.Hmax);
    } else {
        return 0;
    }
}

uint16_t comp_y(pinfo dinfo, uint16_t frame_y, uint8_t comp_id){
    
    JIF_FRAME_COMPONENT * p = frame_comp(dinfo, comp_id);
    
    if(p){
        return ceil( (double)frame_y * p->V / dinfo->frame.Vmax);
    } else {
        return 0;
    }
}

/* private: read top level tables|misc. */
bool dec_read_a_tbl_misc(pinfo dinfo, JIF_SCANNER *s){
    JIF_MARKER m = jif_current_marker(s);
    printf("%x @%llu\n", m, jif_get_offset(s));

    switch (m) {
        case M_DQT:
            printf("%x @%llu DQT\n", m, jif_get_offset(s));
        {   uint16_t Lq = jif_scan_2_bytes(s);
            uint16_t offset = 2;

            while (offset < Lq) {
                byte b = jif_scan_next_byte(s); offset++;
                uint8_t Pq = ( b & 0xF0 )  >> 4 ;
                uint8_t Tq = b & 0x0F;
                
                printf(" > define quantization table @%llu Precision %u, No. %u\n", jif_get_offset(s), Pq == 0 ? 8 : 16, Tq);
                
                dinfo->tQ[Tq].precison = ( 0 == Pq ) ? 8 : 16;
                
                for (int i = 0 ; i < 64 ; i++){
                    if ( 0 == Pq ){
                        uint8_t c = jif_scan_next_byte(s); offset++;
                        dinfo->tQ[Tq].Q[i] = c;
                    } else /* 1 == Pq */ {
                        uint16_t c = jif_scan_2_bytes(s); offset += 2;
                        dinfo->tQ[Tq].Q[i] = c;
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
                uint8_t Tc = ( b & 0xF0 )  >> 4 ;       /* table class 0: DC, 1: AC */
                uint8_t Th = b & 0x0F;
                
                printf(" > define Huffman table @%llu Type %u, No. %u\n", jif_get_offset(s), Tc, Th);
                
                JTBL_HUFF * th = dinfo->tH[Tc][Th];
                memset(th, 0, sizeof(JTBL_HUFF));
                
                th->table_class = Tc;
                
                for(int i = 1; i <= 16; i++){
                    th->bits[i] = jif_scan_next_byte(s); offset++;
                }
                
                int k = 0;
                for(int i = 1; i <= 16; i++){
                    for(int j = 0; j < th->bits[i]; j++){
                        jhuff_set_val(th, k++, jif_scan_next_byte(s)); offset++;
                    }
                }
                
                /* gen huffman table */
                jhuff_gen_decode_tbls(th);
            }
            
        }
            break;
        case M_DAC:
            printf("%x @%llu DAC\n", m, jif_get_offset(s));
            break;
        case M_DRI:
            printf("%x @%llu DRI\n", m, jif_get_offset(s));
        {   uint16_t Lr = jif_scan_2_bytes(s);
            uint16_t offset = 2;
            if ( 4 != Lr ) {err("error DRI length\n");}
            dinfo->scan.Ri = jif_scan_2_bytes(s);  offset+=2; /* restart interval is enabled if > 0 */
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
            printf(" > skipped to offset @%llu\n", jif_get_offset(s));
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
    JIF_MARKER m = jif_current_marker(s);
    switch (m) {
        case M_SOF0:
            dinfo->is_dct_based = true;
            dinfo->frame.data_unit_X = 8;
            dinfo->frame.data_unit_Y = 8;
            
            dinfo->is_use_arithmetic_coding = false;    /* huffman */
            break;
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
            /*  TODO: now only support baseline DCT image. */
            printf("%x @%llu SOF%d\n", m, jif_get_offset(s), m-M_SOF0);
            return false;
            break;
        default:
            printf("%x @%llu NOT M_SOFx\n", m, jif_get_offset(s));
            return false;
            break;
    }
    printf("%x @%llu SOF%d\n", m, jif_get_offset(s), m-M_SOF0);
    dinfo->frame.Lf = jif_scan_2_bytes(s);
    dinfo->frame.P = jif_scan_next_byte(s);    /* sample precision of components in frame */
    dinfo->frame.Y = jif_scan_2_bytes(s);
    dinfo->frame.X = jif_scan_2_bytes(s);
    dinfo->frame.Nf = jif_scan_next_byte(s);
    
    printf(" > %d x %d, %d components.\n", dinfo->frame.X, dinfo->frame.Y, dinfo->frame.Nf);
    
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
        printf(" > components %d  H %d  V %d.\n", dinfo->frame.comps[i].C, H, V);
    }
    
    return true;
}

bool dec_read_DNL(pinfo dinfo, JIF_SCANNER *s ){
    uint16_t Ld = jif_scan_2_bytes(s);
    uint16_t offset = 2;
    if ( 4 != Ld ){
        err("error reading DNL\n");
        return false;
    }
    uint8_t NL = jif_scan_2_bytes(s); offset+=2;   /* number of image component in scan */
    if( 0 == dinfo->frame.Y ){
        dinfo->frame.Y = NL;
    } else {
        err("err: DNL: redefine max number of lines in image.\n");
        return false;
    }
    return true;
}

void * dec_update_img_dimensions (pinfo dinfo){
    JIF_FRAME_COMPONENT * p;
    
    JIF_FRAME * f = &dinfo->frame;
    
    for(int i = 0; i < f->Nf; i++){
        p = &(dinfo->frame.comps[i]);
        if (f->Hmax < p->H){
            f->Hmax = p->H;
        }
        if (f->Vmax < p->V){
            f->Vmax = p->V;
        }
    }
    
    /* only new | reset | realloc dinfo->img here */
    if(!dinfo->img){
        dinfo->img = jimg_new(dinfo->frame.X,
                              dinfo->frame.Y,
                              dinfo->frame.P);
        if(!dinfo->img)
            return 0 ;
    }
    
    uint16_t cX, cY;
    
    for(int i = 0; i < dinfo->frame.Nf; i++){
        p = &dinfo->frame.comps[i];
        cX = comp_x(dinfo, f->X, p->C) +1;
        cY = comp_y(dinfo, f->Y, p->C) +1;
        
        if(!(jimg_set_component(dinfo->img, p->C, cX, cY)))
           return 0;
    }
    return dinfo->img;
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
bool j_dec_read_jpeg_header(pinfo dinfo){
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
        switch (jif_current_marker(s_soi)) {
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
            if(dec_update_img_dimensions(dinfo)){
                if (0 != dinfo->frame.Y){
                    success = true;
                } else {
                    if(jif_scan_next_maker_of(M_DNL, s_soi))
                        if(dec_read_DNL(dinfo, s_soi))
                            if(dec_update_img_dimensions(dinfo))
                                success = true;
                }
                break;
            }
        }
    }
    
    jif_del_scanner(s_soi);
    return success;
}

bool dec_read_scan_header(pinfo dinfo, JIF_SCANNER * s){
    JIF_MARKER m = jif_current_marker(s);
    if( M_SOS != m ){
        return false;
    }
    printf("%x @%llu SOS\n", m, jif_get_offset(s));
    
    uint16_t Ls = jif_scan_2_bytes(s);
    uint16_t offset = 2;
    
    uint8_t Ns = jif_scan_next_byte(s); offset++;   /* number of image component in scan */
    dinfo->scan.Ns = Ns;
    dinfo->scan.comps = realloc(dinfo->scan.comps,
                                Ns * sizeof(JIF_SCAN_COMPONENT));
    byte b;
    int Cs;
    for(int j=0; j < Ns; j++){
        Cs =jif_scan_next_byte(s); offset++;   /* Scan component selector */
        dinfo->scan.comps[j].Cs = Cs;
        b = jif_scan_next_byte(s); offset++;
        dinfo->scan.comps[j].Td = ( b >> 4 );    /* Specifies one of four possible DC entropy coding table */
        dinfo->scan.comps[j].Ta = ( 0x0f & b );  /* Specifies one of four possible AC entropy coding table */
    }
    dinfo->scan.Ss = jif_scan_next_byte(s); offset++;
    dinfo->scan.Se = jif_scan_next_byte(s); offset++;
    b = jif_scan_next_byte(s); offset++;
    dinfo->scan.Ah = ( b >> 4 );
    dinfo->scan.Al = ( 0x0f & b );
    while (offset < Ls) {
        jif_scan_next_byte(s); offset++;
    }
    
    return true;
}

JERR dec_decode_data_unit(pinfo dinfo, JIF_SCANNER * s,
                          unsigned int sj, unsigned int du_x, unsigned int du_y ){
    
    if ( dinfo->is_dct_based ){
        /* Sample precision (in bits) is read from frame header.
         *  8, 12 (DCT), 2~16(lossless).
         *
         * Here we only support baseline DCT which is 8 bit, that is ,
         * the image component sample is 8 bit.
         * the shifted sample is 8 bit (signed, 2's complement representation)
         * as the input of DCT .
         *
         * There is thesis investigated and concluded that,
         * within the context of JPEG  compression level  50,
         * savings can be achieved by using 20 bit (summation),
         * and 12 bit (multiplication) fixed point numbers for DCT.
         *
         * So during DCT float is used for sum and multiplication
         * inside DCT subroutine, for coding here.
         *
         * The quantized DCT coeffecient is 8 ~ 12 bit signed.
         * the huffman encoded -> decoded
         * (signed, 2's complement representation)
         * the dequantized value is 8 ~ 12 bit signed.
         * the shifted value is 8 ~ 12 bit unsigned.
         * ( should be shifted into 0 ~ 255 )
         *
         * So int16_t is used outside of DCT subroutine, which is enough to
         * represent 8, 12 bit signed value. (coeff_t)
         *
         * Finally, shift and clampe IDCT output to sample precision.
         * (0~255 for 8 bit, 0~ 4095 for 12 bit.)
         *
         */
        coeff_t ZZ[DCTSIZE] = {0};
        
        /* decode DC coeff, using DC table specified in scan header. */
        JIF_SCAN_COMPONENT * sp = &dinfo->scan.comps[sj];
        
        huffval_t t = 0;
        coeff_t v = 0, diff = 0;
        
        /* DC table is type 0 */
        JERR e = jhuff_decode(dinfo->tH[0][sp->Td], s, &t);
        if ( JERR_NONE != e){
            return e;
        }
        e = jhuff_receive(t, s, &v);
        if ( JERR_NONE != e){
            return e;
        }
        
        diff = jhuff_extend(v, t);
        ZZ[0] = sp->PRED + diff;
        sp->PRED = ZZ[0];
        
        /* decode AC coeff, using AC table specified in scan header. */

        for (unsigned int K = 1; K < DCTSIZE; K++){
            /* decode RS */
            huffval_t RS = 0;
            e = jhuff_decode(dinfo->tH[1][sp->Ta], s, &RS); /* AC : type 1 huffman table */
            if ( JERR_NONE != e){
                return e;
            }
            huffval_t SSSS = RS % 16; /* 4-bit amplitude category */
            huffval_t RRRR = RS >> 4; /* 4-bit run length */
            huffval_t R = RRRR;
            
            if ( 0 == SSSS ) {
                if ( 15 == R ) {    /* RS == 0xF0: (ZRL) a run length of 15 zero coefficients followed by a coefficient of zero amplitude */
                    K += 15;
                    continue;
                } else {    /* N/A (in baseline) or EOB : all remaining coeff is left 0 */
                    break;
                }
            }
            
            K += R;
            
            /* decode ZZ(K) */
            coeff_t v = 0;
            e = jhuff_receive(SSSS, s, &v);
            if ( JERR_NONE != e){
                return e;
            }
            ZZ[K] = jhuff_extend(v, SSSS);
        }
        
        /* dequantize using table destination specified in the frame header. */
        //Annex A
        JIF_FRAME_COMPONENT * cp = frame_comp(dinfo, dinfo->scan.comps[sj].Cs);
        jquant_dequant(&(dinfo->tQ[cp->Tq]), ZZ);
        
        /* calculate 8 × 8 inverse DCT. */
        //A.3.3
        coeff_t IDCT[DCTWIDTH][DCTWIDTH] = {0};
        j_idct_ZZ(IDCT, ZZ, dinfo->cos_array);
        
//        j_ZZ_dbg(ZZ);
        
        /* level shift after IDCT */
        uint16_t y, x;
        uint16_t min = 0, max = (1 << (dinfo->frame.P)) -1;
        for (y=0; y<DCTWIDTH; y++) {
            for (x=0; x<DCTWIDTH; x++) {
                IDCT[y][x] += 1 << (dinfo->frame.P - 1);
                if( IDCT[y][x] < min){
                    IDCT[y][x] = min;
                } if ( max < IDCT[y][x] ){
                    IDCT[y][x] = max;
                }
            }
        }
        
        /* write to img */
        uint16_t sx, sy;    /* sample x, y */
        for (y=0; y<DCTWIDTH; y++) {
            for (x=0; x<DCTWIDTH; x++) {
                sx = du_x * dinfo->frame.data_unit_X + x;
                sy = du_y * dinfo->frame.data_unit_X + y;
                if (! jimg_X(dinfo->img) ){
                    /* During the fist scan , if frame.Y is unknown,
                     jimg_set_component before writing new sample point.
                     */
                    if(!jimg_set_component(dinfo->img,
                                           sp->Cs,
                                           comp_x(dinfo, jimg_X(dinfo->img), sp->Cs),
                                           comp_y(dinfo, sy+1, sp->Cs))){
                        return JERR_SET_COMPONENT;
                    }
                }
                jimg_write_sample(dinfo->img,
                                  sp->Cs,
                                  sx,
                                  sy,
                                  IDCT[y][x]);
            }
        }
    }
    
    return JERR_NONE;
}

/* comp > block (H x V in MCU) > data unit > sample */
JERR dec_decode_MCU(pinfo dinfo, JIF_SCANNER * s){
    JERR e = JERR_NONE;
    
    for ( int sj = 0; sj < dinfo->scan.Ns; sj++){
        JIF_SCAN_COMPONENT sp = dinfo->scan.comps[sj];
        JIF_FRAME_COMPONENT * cp = frame_comp(dinfo, sp.Cs);
        
        int du_x, du_y; /* data unit (not sample) x, y within component */
        int mcu_x, mcu_y; // debug
        
        for (int v = 0; v < cp->V; v++){
            for (int h = 0; h < cp->H; h++){
                mcu_x = (dinfo->scan.m % dinfo->scan.X_MCU);
                mcu_y= (dinfo->scan.m / dinfo->scan.X_MCU);
                
                du_x =  mcu_x * cp->H + h;    /* data unit x */
                du_y = mcu_y * cp->V + v;
                
                e = dec_decode_data_unit(dinfo, s, sj, du_x, du_y);
                if (JERR_NONE != e){
                    return e;
                }
            }
        }
    }
    
    return JERR_NONE;
}

JERR dec_decode_restart_interval(pinfo dinfo, JIF_SCANNER * s){
    JERR e = JERR_NONE;
    
    /* reset for following decoding */
    if(dinfo->is_dct_based){
        for(int j = 0; j < dinfo->scan.Ns; j++){
            dinfo->scan.comps[j].PRED = 0;
        }
    }
    
    for(int i=0; i < dinfo->scan.Ri; i++ ){
        
        e = dec_decode_MCU(dinfo, s);
        if (JERR_NONE != e){
            return e;
        }
        
        ++dinfo->scan.m;
    }
    
    return JERR_NONE;
}

JERR dec_decode_scan(pinfo dinfo, JIF_SCANNER * s){
    JERR e = JERR_NONE;
    
    if(!dec_read_scan_header(dinfo, s)){
        return JERR_BAD_SCAN_HEADER;
    }
    
    /* calculate parameters for decoding
     ===================================*/
    
    /* This is the tricky part:
     
     At the encoding side, the image is devided into component.
     
     At the decoding side, the image is defined by component,
     ralated to its components with each sampling factors.
     (So, all component is covered with same number of MCU in row / coloumn.)
     Image size is largest component, X, Y may from different component.
     
     
     A scan contains a _complete_ encoding of one or more image components.
     
     But frame Y may not known at 1st scan, in this case, DNL is expected
     after 1st scan, while some component is completed in 1st scan.
     We expand img.component Y every time new line is needed, and call
     update_img() to resize lines inside img.
     
     In either case, we decode MCU and depend on low level nextbit() to raise
     exception on marker DNL, and we test RST marker after every scan->Ri MCU.
     
     If marker is not DNL nor RST, then it might be EOI, return for
     upper level to test (because this function is decode_a_scan() ).
     
     */
    
    JIF_SCAN_COMPONENT * sc;
    JIF_FRAME_COMPONENT * fc;
    JIMG_COMPONENT * ic;
    
    dinfo->scan.Nb = 0;     /* count of data units in MCU */
    dinfo->scan.X_MCU = 0;
    dinfo->scan.Y_MCU = 0;
    dinfo->scan.total_MCU = 0;
    for(int j = 0; j < dinfo->scan.Ns; j++){
        
        sc = &dinfo->scan.comps[j];
        fc = frame_comp(dinfo, sc->Cs);
        ic = jimg_get_component(dinfo->img, sc->Cs);
        
        dinfo->scan.Nb += fc->H * fc->V;
        
        /* prepare for decoding DC */
        dinfo->scan.comps[j].PRED = 0;
    }
    
    if( dinfo->scan.Ns > 1 && dinfo->scan.Nb > 10) {
        printf("err: Too big MCU size %d ( H * V  should < 10 ).\n", dinfo->scan.Nb);
        return JERR_BAD_SCAN_HEADER;
    }
    
    /* do decoding
     =========================== */
    s->bit_cnt = 0; // ?
    
    dinfo->scan.m = 0; /* number MCU decoded */
    
    /* row # MCU = ceiling( max component width / MCU width (in sample) ) */
    
    if( !dinfo->scan.X_MCU ){  /* only needed to calculate once in a scan */
        dinfo->scan.X_MCU = ceil( (double)dinfo->frame.X / dinfo->frame.data_unit_X / dinfo->frame.Hmax);
    }
    
    if( dinfo->frame.Y && !dinfo->scan.Y_MCU){
        dinfo->scan.Y_MCU = ceil( (double)dinfo->frame.Y / dinfo->frame.data_unit_Y / dinfo->frame.Vmax);
        dinfo->scan.total_MCU = dinfo->scan.X_MCU * dinfo->scan.Y_MCU;
    }
    
    if (!dinfo->scan.X_MCU){
        return JERR_MISSING_ROW_MCU_COUNT;
    }
    
    if (dinfo->scan.Y_MCU){
        /* scan size in MCU is known, decode all of MCU. */
        
        if ( dinfo->scan.Ri > 0 ){ /* restart marker (resync mechanism) is enabled.*/
            
            uint32_t RST_count = dinfo->scan.total_MCU / dinfo->scan.Ri;
            for(uint32_t rc = 0; rc < RST_count; rc++){
                
                e = dec_decode_restart_interval(dinfo, s);
                
                if ( JERR_NONE != e ){
                    return e;
                }
                
                /* here a RST marker is expected */
                
                if(jif_scan_next_marker(s)){
                    JIF_MARKER m = jif_current_marker(s);
                    if ( M_RST0 <= m && m <= M_RST7 ){
                        int n = (dinfo->scan.m / dinfo->scan.Ri - 1) % 8;
                        if( (m - M_RST0) == n){
                            s->bit_cnt = 0;
                            continue;
                        }
                    }
                }
                
                if ( rc < RST_count -1){
                    /* Missing RST marker, optinoal resync handling here, modify dinfo->scan.m */
                    err("err: miss RST marker.\n");
                    return JERR_MISS_M_RST;
                }
            }
        }
        
        /* Decode MCU without RST. */
        
        while(dinfo->scan.m < dinfo->scan.total_MCU){
            e = dec_decode_MCU(dinfo, s);
            if (JERR_NONE != e){
                return e;
            }
            ++dinfo->scan.m;
        }
        
    } else {
        /* scan Y is not known, decode first scan. (untilDNL marker) */
        
        if ( dinfo->scan.Ri > 0 ){ /* restart marker (resync mechanism) is enabled.*/
            
            while(true) {
                e = dec_decode_restart_interval(dinfo, s);
                
                if ( JERR_NONE != e ){
                    return e;
                }
                
                /* here a RST marker is expected */
                
                if(jif_scan_next_marker(s)){
                    JIF_MARKER m = jif_current_marker(s);
                    if ( M_RST0 <= m && m <= M_RST7 ){
                        if( (m - M_RST0) == (dinfo->scan.m / dinfo->scan.Ri - 1) % 8 ){
                            s->bit_cnt = 0;
                            continue;
                        }
                    }
                }
                
                /* Missing RST marker, optinoal resync handling here, modify dinfo->scan.m */
                err("err: miss RST marker.\n");
                return JERR_MISS_M_RST;
            }
        }
        
        /* Decode MCU without RST. */
        
        while(true){
            e = dec_decode_MCU(dinfo, s);
            if (JERR_NONE != e){
                return e;
            }
            ++dinfo->scan.m;
        }
    }
    
    return e;
}

/* decode multiple scan of a SOF */
unsigned int dec_decode_multi_scan(pinfo dinfo, JIF_SCANNER * s){
    dinfo->frame.scan_count = 0;
    JERR e = JERR_NONE;
    while(dec_read_tables_misc(dinfo, s) && M_SOS == jif_current_marker(s)) {
        
        e = dec_decode_scan(dinfo, s);
        
        switch (e) { /* error handling */
            case JERR_NONE:
                ++dinfo->frame.scan_count;
                break;
            case JERR_HUFF_NEXTBIT_DNL:
                /* handle DNL
                 which is expected only after 1st scan,
                 while frame.Y is not defined in frame header. */
                if( 0 == dinfo->frame.Y && M_DNL == jif_current_marker(s)){
                    printf("%x @%llu DNL\n", M_DNL, jif_get_offset(s));
                    if(dec_read_DNL(dinfo, s)){
                        dec_update_img_dimensions(dinfo);
                        continue;
                    }
                }
                break;
            default: /* error unknown */
                if (M_EOI == jif_current_byte(s)){
                    /* EOI or more scan (after 1st scan) */
                    printf("%x @%llu EOI\n", M_EOI, jif_get_offset(s));
                    e = JERR_NONE;
                    return ++dinfo->frame.scan_count;
                }
                break;
        }
    }
    
    return dinfo->frame.scan_count;
}

unsigned int dec_decode_multiple_image(pinfo dinfo, JIF_SCANNER * s){
    unsigned int img_counter = 0;
    while( jif_scan_next_maker_of(M_SOI, s) ){
        dinfo->scan.Ri = 0;  /* SOI disable restart interval. */
        
        /* read tables | misc. */
        if (!dec_read_tables_misc(dinfo, s)){
            err("> Error reading tables|misc." );
            continue;
        }
        
        /* read markers before SOS marked scans */
        switch (jif_current_marker(s)) {
            case M_EOI:
                dinfo->frame.mode = JIF_FRAME_MODE_ABBR_TABLE;
                err("> Do not support mode_abbr_table yet." );
                break;
            case M_DHP:
                dinfo->frame.mode = JIF_FRAME_MODE_HIERARCHICAL;
                err("> Do not support mode_hierarchical yet." );
                break;
            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
                if(!dec_read_sof(dinfo, s))     /* frame.Y may not present */
                    break;
                
                if(!dec_update_img_dimensions(dinfo))
                    break;
                
                if( 1 <= dec_decode_multi_scan(dinfo, s)){
                    img_counter ++;
                }
                break;
            default:
                break;
        }
    }
    printf(">> dec_decode_multiple_image() %u\n", img_counter);
    return img_counter;
}

bool j_dec_decode(pinfo dinfo){
    if( 0 == dinfo->src ) {
        return false;
    }
    
    bool success = false;
    
    JIF_SCANNER *s_soi = jif_new_scanner(dinfo->src, dinfo->src_size);
    
    /* Lv 1 : decode multiple image (SOI ~ EOI) */
    if ( 1 <= dec_decode_multiple_image(dinfo, s_soi)){
        success = true;
    }
    
cleanup:
    jif_del_scanner(s_soi);
    return success;
}

JIMG * j_info_get_img(pinfo dinfo){
    return dinfo->img;
}

JERR j_info_get_error(pinfo dinfo){
    return dinfo->err;
}

struct _jbmp_info {
    uint16_t    width;
    uint16_t    height;
    uint8_t     bits_per_pixel;
    uint8_t     bits_per_component;
    uint16_t    bytes_per_row;
};


JBMP_INFO * jbmp_new(void){
    return malloc(sizeof(JBMP_INFO));
}

void j_dec_make_RGBA32(pinfo dinfo, void * dst){
    JBMP_INFO * bmp = jbmp_new();
    if(!bmp) return;
    
    bmp->width = dinfo->frame.X;
    bmp->height = dinfo->frame.Y;
    bmp->bits_per_component = 8;
    bmp->bits_per_pixel = 32;
    bmp->bytes_per_row = 4 * bmp->width;
    
    uint32_t * data = (uint32_t *)dst;
    uint32_t pixel;
    
    int c = dinfo->frame.Nf;
    
    uint8_t comp_i;
    int cy, cx, bi;
    if ( 1 == c ) {
        comp_i = dinfo->frame.comps[0].C;
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i = 0; i < bmp->width; i++ ){
                bi = j * bmp->width + i;
                cy = comp_y(dinfo, j, comp_i);
                cx = comp_x(dinfo, i, comp_i);
                pixel = 0;
                for ( int k = 0; k < 3; k++ ){
                    pixel += jimg_get_sample(dinfo->img, comp_i, cx, cy) >> (dinfo->frame.P - 8);  /* R */
                    pixel <<= 8;
                }
                pixel += 0x00;  /* A */
                data[bi] = pixel;
            }
        }
    } else if ( 3 == c ) {
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i=0 ; i < bmp->width; i++){
                bi = j * bmp->width + i;
                pixel = 0;
                for ( int k = 0; k < c; k++ ){
                    comp_i = dinfo->frame.comps[k].C;
                    cy = comp_y(dinfo, j, comp_i);
                    cx = comp_x(dinfo, i, comp_i);
                    pixel += jimg_get_sample(dinfo->img, comp_i, cx, cy) >> (dinfo->frame.P - 8);  /* R */
                    pixel <<= 8;
                }
                pixel += 0x00;  /* A */
                data[bi] = jYCbCrA2RGBA(pixel);
            }
        }
    }
}

