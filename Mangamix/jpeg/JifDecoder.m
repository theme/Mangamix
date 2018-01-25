//
//  JifDecoder.m
//  Mangamix
//
//  Created by theme on 23/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#import "JifDecoder.h"

@implementation JifDecoder

- (CGImageRef) decodeJifData:(NSData*) jifData{
    pinfo dinfo = j_dec_new();
    j_dec_set_src_array((unsigned char*)[jifData bytes], [jifData length], dinfo);
    CGImageRef ir = NULL;
    if (j_dec_decode(dinfo)){
    
        
        // TODO : construct CGImage
        size_t width = j_info_get_width(dinfo);
        size_t height = j_info_get_height(dinfo);
        size_t bitsPerComponent;
        size_t bitsPerPixel;
        size_t bytesPerRow;
        CGColorSpaceRef space;
        CGBitmapInfo bitmapInfo;
        CGDataProviderRef provider;
        const CGFloat *decode;
        bool shouldInterpolate;
        CGColorRenderingIntent intent;
        ir = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, space, bitmapInfo, provider, decode, shouldInterpolate, intent);
    
    }
    j_dec_destroy(dinfo);
    return ir;
}

@end

