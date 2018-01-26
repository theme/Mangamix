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
        size_t width = j_info_get_width(dinfo);     /* in pixel for image */
        size_t height = j_info_get_height(dinfo);
        size_t bitsPerComponent = 8;
        size_t bitsPerPixel = 24;
        size_t bytesPerRow = 150;
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipLast;
        CGDataProviderRef provider = CGDataProviderCreateWithData(dinfo, j_info_get_img_data(dinfo), j_info_get_img_data_size(dinfo), j_info_release_img_data);
        const CGFloat *decode = NULL;   /* do not map color */
        bool shouldInterpolate = true;  /* Core Graphics should apply a pixel-smoothing algorithm to the image, when output device with higher resolution than data. */
        CGColorRenderingIntent intent = kCGRenderingIntentAbsoluteColorimetric;
        ir = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, space, bitmapInfo, provider, decode, shouldInterpolate, intent);
        
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(space);
    }
    j_dec_destroy(dinfo);
    return ir;
}

@end

