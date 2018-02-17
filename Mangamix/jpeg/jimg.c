//
//  jimg.c
//  Mangamix
//
//  Created by theme on 26/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//
//  image
//      *component
//      *component
//          *line
//          *line
//              sample sample sample sample ...


#include "jimg.h"

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

    for( i = 0; i < img->comps_count; i++){
        c = &img->comps[i];
        if (c->cid == comp_id){
            break;
        }
    }
    
    if ( i == img->comps_count ){   /* no such component yet */
        img->comps = realloc(img->comps,
                             (img->comps_count + 1) * sizeof(JIMG_COMPONENT));
        if(img->comps){
            c = &img->comps[img->comps_count];
            c->lines = malloc(0);
            if(!c->lines)
                return 0;
            img->comps_count++;
            c->X = 0;
            c->Y = 0;
            c->cid = comp_id;
        }
    }
    
    if(c){
        uint16_t oy = c->Y;
        
        /* free old lines */
        for(int j = oy -1 ; j > height -1; --j){
            free(c->lines[j]);
        }
        
        /* realloc line pointers */
        c->lines = realloc(c->lines, height * sizeof(JIMG_SAMPLE *));
        if(!c->lines)
            return 0;
        
        /* alloc new lines */
        for(int j = oy; j < height; j++){
            c->lines[j] = malloc(0);
            if(!c->lines[j])
                return 0;
        }
        
        c->Y = height;
        
        /* resize each line width */
        for(int j = 0; j < height; j++){
            c->lines[j] = realloc(c->lines[j], width * sizeof(JIMG_SAMPLE));
            if(!c->lines[j])
                return 0;
        }
        c->X = width;
        return c;
    } else {
        return 0;
    }
}

JIMG_COMPONENT * jimg_get_component(JIMG * img, uint8_t comp_id){
    JIMG_COMPONENT * c;
    int i;
    for( i = 0 ; i < img->comps_count; i++){
        c = &img->comps[i];
        if(c->cid == comp_id){
            return c;
        }
    }
    return 0;
}

JIMG * jimg_write_sample(JIMG * img, uint8_t comp_id, uint16_t x, uint16_t y, uint16_t s){
    uint16_t smax = (1 << img->precision) - 1;
    if ( s < 0 ){
         s = 0;
    } else if ( s > smax){
        s = smax;
    }
    
    JIMG_COMPONENT * c;
    
    for( int i = 0 ; i < img->comps_count; i++){
        c = &img->comps[i];
        if(c->cid == comp_id){
            if ( x < c->X && y < c->Y ){
                c->lines[y][x] = s;
                return img;
            } else return 0;
        }
    }
    
    return 0;
}

void jimg_free(JIMG * img){
    if(img) {
        for( int i = 0 ; i < img->comps_count; i++){
            JIMG_COMPONENT * c = &img->comps[i];
            if(c) {
                for(int j = 0 ; j < c->Y; j++){
                    free(c->lines[j]);
                }
                free(c->lines);
            }
        }
        free(img);
    }
}

JBMP_INFO * jbmp_new(void){
    return malloc(sizeof(JBMP_INFO));
}

void jbmp_make_RGBA32(JIMG * img, void * dst){
    JBMP_INFO * bmp = jbmp_new();
    bmp->width = img->X;
    bmp->height = img->Y;
    bmp->bits_per_component = 8;
    bmp->bits_per_pixel = 32;
    bmp->bytes_per_row = 4 * img->X;
    
    uint32_t * data = (uint32_t *)dst;
    uint32_t pixel;
    
    int c = img->comps_count;
    
    JIMG_COMPONENT * cp;
    int cy, cx, bi;
    if ( 1 == c ) {
        cp = &img->comps[0];
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i = 0; i < bmp->width; i++ ){
                bi = j * bmp->width + i;
                cy = j * cp->Y /  bmp->height;
                cx = i * cp->X / bmp->width;
                pixel = 0;
                for ( int k = 0; k < 3; k++ ){
                    pixel += cp->lines[cy][cx] >> (img->precision - 8);  /* R */
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
                    cp = &img->comps[k];
                    cy = j * cp->Y * 1.0 / bmp->height;
                    cx = i * cp->X * 1.0 / bmp->width;
                    pixel += cp->lines[cy][cx] >> (img->precision - 8); /* R, G, B */
                    pixel <<= 8;
                }
                pixel += 0x00;  /* A */
                data[bi] = jYCbCrA2RGBA(pixel);
            }
        }
    }
}

