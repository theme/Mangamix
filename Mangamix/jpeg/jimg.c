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

JIMG * jimg_set_components(JIMG * img, uint8_t index, uint16_t H, uint16_t V, unsigned int precision){
    JIMG_COMPONENT * newp;
    if ( (newp = realloc(img->comps[index], sizeof(JIMG_COMPONENT)))) {
        img->comps[index] = newp;
        newp->data = realloc(newp->data, sizeof(JIMG_SAMPLE) * img->X * img->Y);
        if(!newp->data) {
            return NULL;
        }
        newp->H = H;
        newp->V = V;
        newp->sample_precision = precision;
        if( false == img->comp_map[index] ){
            img->num_of_components ++;
            img->comp_map[index] = true;
        }
    }
    
    return img;
}


JIMG * jimg_write_sample(JIMG * img, uint8_t index, uint16_t x, uint16_t y, double s){
    JIMG_COMPONENT * cp;
    cp = img->comps[index];
    
    uint16_t smax = 1 << (cp->sample_precision - 1);
    
    if ( s < 0 ){
        cp->data[y * img->Y + x] = 0;
    } else if ( s > smax){
        cp->data[y * img->Y + x] = smax;
    }
    
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

JIMG_BITMAP * jbmp_new(void){
    JIMG_BITMAP * bmp;
    if( (bmp = malloc(sizeof(JIMG_BITMAP)))){
        bmp->data = malloc(0);
        bmp->data_size = 0;
    }
    return bmp;
}

void jbmp_free(JIMG_BITMAP * bmp){
    free(bmp->data);
    free(bmp);
}
