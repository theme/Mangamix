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
    
    [self.imageView setImage: [self imageFromJPG:url]];
}

// decode and show JPEG file
- (NSImage*) imageFromJPG:(NSURL*) url{
    NSFileManager *fm = [NSFileManager defaultManager];
    unsigned long long fileSize = 0;
    NSDictionary *fileAttr = nil;
    fileAttr = [fm attributesOfItemAtPath:url.path error:nil];
    if ( nil != fileAttr ) {
        fileSize = [fileAttr fileSize];
    } else {
        NSLog(@"Error getting file size!");
        return nil;
    }
    //
    if ( ![fm isReadableFileAtPath:url.path] ){
        NSLog(@"Error getting file size!");
        return nil;
    }
//    _imageView.image = [[NSImage alloc] initByReferencingURL:url];
    
    // read jpeg file into buffer
    NSData *fileData;
    fileData = [fm contentsAtPath:url.path];
    
    // decode
    CGImageRef cgi = [[self jifDecoder] decodeJifData:fileData];
    if( cgi ) {
        NSSize s;
        s.height = CGImageGetHeight(cgi);
        s.width = CGImageGetWidth(cgi);
        return [[NSImage alloc] initWithCGImage:cgi size:s];
    } else {
        return NULL;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    self.jifDecoder = [JifDecoder alloc];
    
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

