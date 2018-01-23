//
//  exif.h
//  Mangamix
//
//  Created by theme on 24/01/2018.
//  Copyright © 2018 theme. All rights reserved.
//

#ifndef exif_h
#define exif_h

//  Define some Exif format used in JIF/APP1

#include <stdint.h>

typedef struct TIFF_IFD {
    uint16_t num_fields;
    uint16_t tag;
    uint16_t type;
    uint32_t count;
    uint32_t value_offset;
    uint32_t next_IFD_offset;
} tiff_ifd_t;

typedef enum {
    EXIF_BYTE = 1,
    EXIF_ASCII = 2,
    EXIF_SHORT = 3,
    EXIF_LONG = 4,
    EXIF_RATIONAL = 5,
    EXIF_UNDEFINED = 7,
    EXIF_SLONG = 9,
    EXIF_SRATIONAL = 10
} EXIF_TYPE ;

#endif /* exif_h */
