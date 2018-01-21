//
//  JifDecoder.h
//  Mangamix
//
//  Created by theme on 23/12/2017.
//  Copyright © 2017 theme. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>

#import "jpegdec.h"

// in >> NSData ( Jif file )
// out << CGImageSource ( Reconstructed Image )
@interface JifDecoder : NSObject

//- (CGImageRef) decodeJifData:(NSData*) jifData;

@end
