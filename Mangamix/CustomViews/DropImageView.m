//
//  DropImageView.m
//  Mangamix
//
//  Created by theme on 2018/8/5.
//  Copyright © 2018 theme. All rights reserved.
//

#import "DropImageView.h"

@implementation DropImageView


- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        [self registerForDraggedTypes:[NSImage imageTypes]];
    }
    return self;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
    if([NSImage canInitWithPasteboard:[sender draggingPasteboard]]
       && [sender draggingSourceOperationMask] & NSDragOperationCopy) {
        return NSDragOperationCopy;
    }
    return NSDragOperationNone;
}

//- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
//    NSLog(@"Dragging");
//    return NSDragOperationCopy;
//}

- (void)draggingEnded:(id<NSDraggingInfo>)sender {
    NSLog(@"Endded");
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
    
    NSLog(@"Exited");
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

@end
