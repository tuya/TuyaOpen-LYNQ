#ifndef __ZBAR_CONFIG_H__
#define __ZBAR_CONFIG_H__

#include <stdint.h>


/* include/config.h.  Generated from config.h.in by configure.  */
/* include/config.h.in.  Generated from configure.ac by autoheader.  */

/* whether to build support for Code 128 symbology */
#define ENABLE_CODE128 

/* whether to build support for Code 39 symbology */
#define ENABLE_CODE39 

/* whether to build support for EAN symbologies */
#define ENABLE_EAN 

/* whether to build support for Interleaved 2 of 5 symbology */
#define ENABLE_I25 

/* whether to build support for PDF417 symbology */
#define ENABLE_PDF417

/* whether to build support for QR Code */
#define ENABLE_QRCODE 1

/* Program major version (before the '.') as a number */
#define ZBAR_VERSION_MAJOR 0

/* Program minor version (after '.') as a number */
#define ZBAR_VERSION_MINOR 10

#ifndef PRIx32
#define 	PRIx32   "lx"
#endif

#endif //__ZBAR_CONFIG_H__


