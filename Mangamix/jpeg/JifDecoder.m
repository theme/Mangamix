//
//  JifDecoder.m
//  Mangamix
//
//  Created by theme on 23/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#import "JifDecoder.h"

#import "jpegdec.h"

@implementation JifDecoder

- (CGImageRef) decodeJifData:(NSData*) jifData{
    pinfo dinfo = j_dec_new();
    j_dec_set_src_array((unsigned char*)[jifData bytes], [jifData length], dinfo);
    CGImageRef ir = NULL;
    
    if (j_dec_decode(dinfo)){
        size_t width = j_info_img_width(dinfo);     /* in pixel for image */
        size_t height = j_info_img_height(dinfo);
        
        JBMP * bmp = jbmp_make_RGB24(dinfo->img, dinfo->bmp);
        
        size_t bitsPerComponent = 8;
        size_t bitsPerPixel = 24;
        size_t bytesPerRow = bmp->width * bitsPerPixel;
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipLast;
        CGDataProviderRef provider = CGDataProviderCreateWithData(dinfo, bmp->data, bmp->data_size, jbmp_release);
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

