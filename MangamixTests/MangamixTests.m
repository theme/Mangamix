//
//  MangamixTests.m
//  MangamixTests
//
//  Created by theme on 03/10/2017.
//  Copyright © 2017 theme. All rights reserved.
//

#import <XCTest/XCTest.h>

#import <ImageIO/ImageIO.h>

#include "jpegdec.h"

@interface MangamixTests : XCTestCase

@end

@implementation MangamixTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testJpegReadHeader {
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *filePath = [bundle pathForResource:@"baseline-standard" ofType:@"jpg"];

    
    // get file size
    NSFileManager *fm = [NSFileManager defaultManager];
    unsigned long long fileSize = 0;
    NSError *err = nil;
    NSDictionary *fileAttr = nil;
    
    fileAttr = [fm attributesOfItemAtPath:filePath error:&err];
    if ( nil != fileAttr ) {
        fileSize = [fileAttr fileSize];
//        NSLog(@"Got file attributes:  %@", fileAttr);
    } else {
        NSLog(@"Error getting file size! %@", err);
    }
    
    XCTAssert( [fm fileExistsAtPath:filePath]);
    XCTAssert( [fm isReadableFileAtPath:filePath] );
    XCTAssert( fileSize > 0 );
    
    if ( [fm isReadableFileAtPath:filePath] ){
        // read file into buffer
        NSData *fileData;
        fileData = [fm contentsAtPath:filePath];
        
        // test : decode jpeg header
        pinfo p = j_dec_new();
        j_dec_set_src_array((byte*)[fileData bytes], fileSize, p);
        if(j_dec_read_header(p)){
            NSLog(@"width %ld, height %ld", j_info_get_width(p), j_info_get_height(p) );
        }
        XCTAssert(j_dec_read_header(p));
        XCTAssert(32 == j_info_get_width(p));
        XCTAssert(30 == j_info_get_height(p));
    }
    
}

//- (void)testByteArray {
//    char array[4] = {0x22, 0xB0, 0x55, 0x6c};
//    char * ptr = array;
//    XCTAssert(ptr[3] == 0x6c);
//}
//
//- (void)testPerformanceExample {
//    // This is an example of a performance test case.
//    [self measureBlock:^{
//        // Put the code you want to measure the time of here.
//    }];
//}

//- (void)testSupportedUTIs {
//    CFArrayRef mySourceTypes = CGImageSourceCopyTypeIdentifiers();
//    CFShow(mySourceTypes);
//    CFArrayRef myDestinationTypes = CGImageDestinationCopyTypeIdentifiers();
//    CFShow(myDestinationTypes);
//}

@end
