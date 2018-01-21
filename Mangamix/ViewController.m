//
//  ViewController.m
//  Mangamix
//
//  Created by theme on 03/10/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#import "ViewController.h"

@implementation ViewController

- (IBAction)chooseImage:(id)sender {

    // This method displays the panel and returns immediately.
    // The completion handler is called when the user selects an
    // item or cancels the panel.
    [self.openImagePanel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSURL*  theDoc = [[self.openImagePanel URLs] objectAtIndex:0];
            [self openImage:theDoc];
            
        }
        
    }];
}

- (void) openImage:(NSURL*) url{
    NSLog(@"%@", [NSString stringWithFormat: @"openImage:(NSURL*) %@", url.absoluteString]);
    
    [self imageFromJPG:url];
}

// decode and show JPEG file
- (void) imageFromJPG:(NSURL*) url{
    NSFileManager *fm = [NSFileManager defaultManager];
    unsigned long long fileSize = 0;
    NSError *err = nil;
    NSDictionary *fileAttr = nil;
    fileAttr = [fm attributesOfItemAtPath:url.path error:&err];
    if ( nil != err ) {
        fileSize = [fileAttr fileSize];
    } else {
        NSLog(@"Error getting file size!");
    }
    //
    if ( [fm isReadableFileAtPath:url.path] ){
        _imageView.image = [[NSImage alloc] initByReferencingURL:url]; // TODO: replace with homebrew decoder
        
        // read file into buffer
        NSData *fileData;
        fileData = [fm contentsAtPath:url.path];
        
        // read file header into probing buffer
        NSData *header = [NSData alloc];
        const unsigned long long BUFLEN = fileSize; // Full buffered image file
        unsigned char* buffer[BUFLEN];
        [fileData getBytes:buffer length:BUFLEN];
        header = [header initWithBytes:buffer length:BUFLEN];
        NSLog(@"%@", [header description]);
        
        // TODO replace header buffer mechanism with NSData method > - (void)getBytes:(void *)buffer range:(NSRange)range;
//        NSRange ra = NSMakeRange(2, 3);
        
        // a decoder class : XTMJpegDecoder
        // Needed: (that NSMutableData can do?)
        // io from file (yes)
        // read a byte, check, read the next (" the next " needs pointer type: NSRange )
        // on encountering a marker, populate a corresponding NSData <- dataWithBytesNoCopy to describe it ( from, to, type )
        // decoder class ( input: data | table, output : data )
        // After: image class : NSImage
        
        
        // Decode JFIF format
        // TODO
        // Maybe two interfaces ( JFIF < JIF  ) is needed
        const unsigned char JFIFheader[] = { 0xff, 0xd8, 0xff, 0xe0 };  // JFIF: FF D8 FF E0
        if(0 == memcmp(header.bytes, JFIFheader, sizeof JFIFheader))
        {
            NSLog(@"> JFIF");
        }
        
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.u
    
    // Create and configure the panel.
    self.openImagePanel = [NSOpenPanel openPanel];
    [self.openImagePanel setCanChooseDirectories:NO];
    [self.openImagePanel setAllowsMultipleSelection:NO];
    [self.openImagePanel setMessage:@"What image do you want to open?"];
    NSLog(@"%@", [self.openImagePanel description]);
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end

