//
//  jimg.c
//  Mangamix
//
//  Created by theme on 26/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#include "jimg.h"


JIMG * jimg_new(void){
    JIMG * img;
    if( (img = malloc(sizeof(JIMG)))){
        img->num_of_components = 0;
        for(int i = 0; i < JMAX_COMPONENTS; i++){
            img->comps[i] = malloc(0);
            img->comp_map[i] = false;
        }
        img->X = 0;
        img->Y = 0;
    }
    return img;
}

JIMG * jimg_set_components(JIMG * img, uint8_t index, uint16_t X, uint16_t Y, unsigned int precision){
    JIMG_COMPONENT * newp;
    if ( (newp = realloc(img->comps[index], sizeof(JIMG_COMPONENT)))) {
        img->comps[index] = newp;
        newp->data = realloc(newp->data, sizeof(JIMG_SAMPLE) * X * Y);
        if(!newp->data) {
            return NULL;
        }
        newp->X = X;
        newp->Y = Y;
        newp->precision = precision;
        if( false == img->comp_map[index] ){
            img->num_of_components ++;
            img->comp_map[index] = true;
        }
    }
    
    return img;
}


JIMG * jimg_write_sample(JIMG * img, uint8_t index, uint16_t x, uint16_t y, double s){
    uint16_t smax = 1 << (img->comps[index]->precision - 1);
    
    if ( s < 0 ){
         s = 0;
    } else if ( s > smax){
        s = smax;
    }
    
    img->comps[index]->data[x * img->Y + y] = s;
    return img;
}

void jimg_free(JIMG * img){
    
    for(int i = 0; i < JMAX_COMPONENTS; i++){
        if( img->comp_map[i] ){
            free(img->comps[i]->data);
            free(img->comps[i]);
        }
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
    
    int c = img->num_of_components;
    JIMG_COMPONENT * comps[c];
    
    for ( int i = 0, n = 0; i < JMAX_COMPONENTS && n <= c; i++){
        if(img->comp_map[i]){
            comps[n++] = img->comps[i];
        }
    }
    
    JIMG_COMPONENT * cp;
    int cpi, bi;
    if ( 1 == c ) {
        cp = comps[0];
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
                    cp = comps[k];
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
