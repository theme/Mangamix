//
//  jerr.h
//  Mangamix
//
//  Created by theme on 16/02/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef jerr_h
#define jerr_h

typedef enum {
    JERR_NONE = 0,
    JERR_MALLOC = -1,  /* not enough memory */
    JERR_UNKNOWN = -2,
    JERR_HUFF_NEXTBIT_DNL = -3,
    JERR_BAD_SCAN_HEADER = -4,
    JERR_MISSING_MCU_COUNT_IN_ROW = -5,
    JERR_SET_COMPONENT = -6
} JERR;


#endif /* jerr_h */
