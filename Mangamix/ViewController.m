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
    
    // Get the main window for the document.
    NSWindow* window = _imageView.window;
    
    // Create and configure the panel.
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setMessage:@"What image do you want to open?"];
    
    // Display the panel attached to the document's window.
    [panel beginSheetModalForWindow:window completionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSArray* urls = [panel URLs];
            
            // Use the URLs to build a list of items to import.
            [self openImage:urls[0]];
        }
        
    }];
}

- (void) openImage:(NSURL*) url{
    NSLog(@"%@", [NSString stringWithFormat: @"openImage:(NSURL*) %@", url.absoluteString]);
    if (!_imageView.image){
        _imageView.image = [[NSImage alloc] initByReferencingURL:url];
    } else {
//        [_imageView.image release]; // not needed because of ARC
        _imageView.image = [[NSImage alloc] initByReferencingURL:url];
        NSLog( @"Image is already loaded." );
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end
