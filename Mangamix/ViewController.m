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

- (void) imageFromJPG:(NSURL*) url{
    NSFileManager *fm = [NSFileManager defaultManager];
    
    //
    if ( [fm isReadableFileAtPath:url.path] ){
        _imageView.image = [[NSImage alloc] initByReferencingURL:url];
        
        NSData *databuffer;
        databuffer = [fm contentsAtPath:url.path];
        
        NSData *header = [NSData alloc];
        const int BUFLEN = 10;
        unsigned char* buffer[BUFLEN];
        [databuffer getBytes:buffer length:BUFLEN];
        header = [header initWithBytes:buffer length:BUFLEN];
        NSLog(@"%@", [header description]);
        

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

