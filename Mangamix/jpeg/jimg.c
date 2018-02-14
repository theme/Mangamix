//
//  jimg.c
//  Mangamix
//
//  Created by theme on 26/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jimg.h"

void free_component(JIMG_COMPONENT * c){
    free(c->data);
    free(c);
}

JIMG * jimg_new(uint16_t width, uint16_t height, uint16_t precision){
    JIMG * img;
    if( (img = malloc(sizeof(JIMG)))){
        img->comps = malloc(0);
        if (img->comps) {
            img->comps_count = 0;
        } else return 0;
        img->X = width;
        img->Y = height;
        img->precision = precision;
        return img;
    } else return 0;
}

JIMG_COMPONENT * jimg_set_component(JIMG * img, uint8_t comp_id, uint16_t width, uint16_t height){
    JIMG_COMPONENT * c = 0;
    int i;
    
    height = height == 0 ? 1 : height;  /* frame.Y may be known after 1st scan */
    
    for( i = 0; i < img->comps_count; i++){
        c = &img->comps[i];
        if (c->cid == comp_id){
            break;
        }
    }
    
    if ( i == img->comps_count ){
        img->comps = realloc(img->comps, (++img->comps_count) * sizeof(JIMG_COMPONENT));
        if(img->comps){
            c = &img->comps[img->comps_count-1];
            c->data = malloc(0);
            if(!c->data)
                return 0;
        }
    }
    
    if(c){
        c->X = width;
        c->Y = height;
        c->data = realloc(c->data, width * height * sizeof(uint16_t));
        if(c->data)
            return c;
    }
    return 0;
}

JIMG * jimg_write_sample(JIMG * img, uint8_t comp_id, uint16_t x, uint16_t y, double s){
    uint16_t smax = 1 << (img->precision - 1);
    if ( s < 0 ){
         s = 0;
    } else if ( s > smax){
        s = smax;
    }
    
    JIMG_COMPONENT * c;
    int i;
    for( i = 0 ; i < img->comps_count; i++){
        c = &img->comps[i];
        if(c->cid == comp_id){
            if ( x <= c->X && y <= c->Y ){
                c->data[y * img->X + x] = s;
                return img;
            } else return 0;
        }
    }
    
    return 0;
}

void jimg_free(JIMG * img){
    int i;
    for( i = 0 ; i < img->comps_count; i++){
        free(img->comps[i].data);
    }
    free(img->comps);
    free(img);
}

JBMP * jbmp_new(void){
    JBMP * bmp;
    if( (bmp = malloc(sizeof(JBMP)))){
        bmp->data = malloc(0);
        bmp->data_size = 0;
    }
    return bmp;
}

void jbmp_make_RGB24(JIMG * img, JBMP * bmp){
    bmp->width = img->X;
    bmp->height = img->Y;
    bmp->bits_per_pixel = 24;
    bmp->bits_per_component = 8;
    bmp->data_size = 3 * img->X * img->Y;
    bmp->data = realloc(bmp->data, bmp->data_size);
    
    int c = img->comps_count;
    
    JIMG_COMPONENT * cp;
    int cpi, bi;
    if ( 1 == c ) {
        cp = &img->comps[0];
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i=0; i < bmp->width; i++ ){
                bi = j * bmp->width + i;
                cpi = j * (cp->Y / bmp->height) + i * (cp->X / bmp->width);
                bmp->data[bi] = cp->data[cpi];     /* R */
                bmp->data[bi+1] = cp->data[cpi];   /* G */
                bmp->data[bi+2] = cp->data[cpi];   /* B */
            }
        }
    } else if ( 3 == c ) {
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i=0; i < bmp->width; i++ ){
                for ( int k = 0; k < c; k++ ){
                    cp = &img->comps[k];
                    bi = j * bmp->width + i;
                    cpi = j * (cp->Y / bmp->height) + i * (cp->X / bmp->width);
                    bmp->data[bi + k] = cp->data[cpi];     /* R, G, B */
                }
            }
        }
    }
}

void jbmp_release(void *info, const void *data, size_t size){
    jbmp_free((JBMP *)data);
}

void jbmp_free(JBMP * bmp){
    free(bmp->data);
    free(bmp);
}
