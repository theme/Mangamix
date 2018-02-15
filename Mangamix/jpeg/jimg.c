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

void free_component(JIMG_COMPONENT * c){
    for(int j = 0 ; j < c->Y; j++){
        free(c->lines[j]);
    }
    free(c->lines);
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

    for( i = 0; i < img->comps_count; i++){
        c = &img->comps[i];
        if (c->cid == comp_id){
            break;
        }
    }
    
    if ( i == img->comps_count ){   /* no such component yet */
        img->comps = realloc(img->comps, (++img->comps_count) * sizeof(JIMG_COMPONENT));
        if(img->comps){
            c = &img->comps[img->comps_count-1];
            c->lines = malloc(0);
            if(!c->lines)
                return 0;
            c->X = 0;
            c->Y = 0;
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
        c->cid = comp_id;
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

JIMG * jimg_write_sample(JIMG * img, uint8_t comp_id, uint16_t x, uint16_t y, double s){
    uint16_t smax = 1 << (img->precision - 1);
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
    for( int i = 0 ; i < img->comps_count; i++){
        free_component(&img->comps[i]);
    }
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
    int cy, cx, bi;
    if ( 1 == c ) {
        cp = &img->comps[0];
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i=0; i < bmp->width; i++ ){
                bi = j * bmp->width + i;
                cy = j * cp->Y /  bmp->height;
                cx = i * cp->X / bmp->width;
                bmp->data[bi] = cp->lines[cy][cx];     /* R */
                bmp->data[bi+1] = cp->lines[cy][cx];   /* G */
                bmp->data[bi+2] = cp->lines[cy][cx];   /* B */
            }
        }
    } else if ( 3 == c ) {
        for (int j = 0 ; j < bmp->height; j++) {
            for( int i=0; i < bmp->width; i++ ){
                for ( int k = 0; k < c; k++ ){     /* R, G, B */
                    cp = &img->comps[k];
                    bi = j * bmp->width + i;
                    cy = j * cp->Y / bmp->height * cp->X;
                    cx = i * cp->X / bmp->width;
                    bmp->data[bi + k] = cp->lines[cy][cx];
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
