//
//  ViewController.h
//  Mangamix
//
//  Created by theme on 03/10/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "JifDecoder.h"

@interface ViewController : NSViewController

@property (weak) IBOutlet NSImageView *imageView;
@property (strong) NSOpenPanel *openImagePanel;
@property (strong) JifDecoder *jifDecoder;

@end

