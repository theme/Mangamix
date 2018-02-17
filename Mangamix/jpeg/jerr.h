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
    JERR_HUFF_NEXTBIT_M_UNKNOWN = -4,
    JERR_BAD_SCAN_HEADER = -5,
    JERR_MISSING_ROW_MCU_COUNT = -6,
    JERR_SET_COMPONENT = -7,
    JERR_MISS_M_RST = -8
} JERR;

#endif /* jerr_h */
