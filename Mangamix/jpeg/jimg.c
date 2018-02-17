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

struct _jimg_component {
    uint8_t     cid;       /* a unique label to the ith component */
    uint16_t    X;          /* not for external use, no sampling factor here. */
    uint16_t    Y;
    JIMG_SAMPLE **lines;    /* the decoded image data */
};

struct _jimg {
    uint16_t        X;      /* maximum component's width */
    uint16_t        Y;
    unsigned int    precision;        /* DCT: 8,12; those lossless: 2 ~ 16 */
    JIMG_COMPONENT  *comps;
    int             comps_count;    /* number of component */
};

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

uint16_t jimg_X(JIMG * img){
    return img->X;
}
uint16_t jimg_Y(JIMG * img){
    return img->Y;
}
uint8_t jimg_component_count(JIMG * img){
    return img->comps_count;
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


JIMG_SAMPLE jimg_get_sample(JIMG * img, uint8_t comp_id, uint16_t x, uint16_t y ){
    JIMG_COMPONENT * c;
    for( int i = 0 ; i < img->comps_count; i++){
        c = &img->comps[i];
        if(c->cid == comp_id){
            if ( x < c->X && y < c->Y ){
                return c->lines[y][x];
            }
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
