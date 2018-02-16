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
        
        int width = dinfo->img->X;     /* in pixel for image */
        int height = dinfo->img->Y;
        int bitsPerComponent = 8;
        int bitsPerPixel = 32;
        int bytesPerRow = 4 * width;
        int data_size = bytesPerRow * height;
        
        NSMutableData * bmpData = [NSMutableData dataWithLength:data_size];
        void * data = [bmpData mutableBytes];
        
        if ( bmpData.length > 0 && data) {
            jbmp_make_RGBA32(dinfo->img, data);
//            for (int i =0; i < data_size; i++){
//                ((uint32 *)data)[i/4] = 0xFF000000;
//            }
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipLast;
            CGDataProviderRef provider = CGDataProviderCreateWithData(NULL,[bmpData bytes], data_size, NULL);
            const CGFloat *decode = NULL;   /* do not map color */
            bool shouldInterpolate = true;  /* Core Graphics should apply a pixel-smoothing algorithm to the image, when output device with higher resolution than data. */
            CGColorRenderingIntent intent = kCGRenderingIntentAbsoluteColorimetric;
            ir = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, space, bitmapInfo, provider, decode, shouldInterpolate, intent);
            
            CGDataProviderRelease(provider);
            CGColorSpaceRelease(space);
        }
    }
    j_dec_free(dinfo);
    return ir;
}

@end

