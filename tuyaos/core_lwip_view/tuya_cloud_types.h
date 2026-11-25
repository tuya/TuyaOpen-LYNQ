/**
 * @file tuya_cloud_types.h
 * @brief TAL type alias shim for tuyaopen-core and service components on L511.
 *
 * Wraps the platform/shared tuya_cloud_types.h via #include_next and appends
 * the legacy TAL typedefs/macros (SIZE_T, INLINE, INT_T, VOID_T, ...) expected
 * by tuyaopen-core without duplicating or shadowing platform-specific types.
 */
#ifndef __L511_CORE_TUYA_CLOUD_TYPES_SHIM__
#define __L511_CORE_TUYA_CLOUD_TYPES_SHIM__

#include_next "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long long      DLONG_T;
typedef DLONG_T       *PDLONG_T;
typedef float          FLOAT_T;
typedef FLOAT_T       *PFLOAT_T;
typedef int            INT_T;
typedef int           *PINT_T;
typedef void          *PVOID_T;
typedef char           CHAR_T;
typedef char          *PCHAR_T;
typedef signed char    SCHAR_T;
typedef unsigned char  UCHAR_T;
typedef short          SHORT_T;
typedef unsigned short USHORT_T;
typedef short         *PSHORT_T;
typedef long           LONG_T;
typedef unsigned long  ULONG_T;
typedef long          *PLONG_T;
typedef unsigned char  BYTE_T;
typedef BYTE_T        *PBYTE_T;
typedef uint32_t       UINT_T;
typedef uint32_t      *PUINT_T;
typedef int64_t        INT64_T;
typedef INT64_T       *PINT64_T;
typedef uint64_t       UINT64_T;
typedef UINT64_T      *PUINT64_T;
typedef uint32_t       UINT32_T;
typedef UINT32_T      *PUINT32_T;
typedef int            INT32_T;
typedef int           *PINT32_T;
typedef short          INT16_T;
typedef INT16_T       *PINT16_T;
typedef unsigned short UINT16_T;
typedef UINT16_T      *PUINT16_T;
typedef signed char    INT8_T;
typedef INT8_T        *PINT8_T;
typedef unsigned char  UINT8_T;
typedef UINT8_T       *PUINT8_T;
typedef double         DOUBLE_T;
typedef unsigned short WORD_T;
typedef WORD_T        *PWORD_T;
typedef unsigned int   DWORD_T;
typedef DWORD_T       *PDWORD_T;
typedef size_t         SIZE_T;

#ifndef CONST
#define CONST const
#endif
#ifndef STATIC
#define STATIC static
#endif
#ifndef INLINE
#define INLINE inline
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef VOID_T
#define VOID_T void
#endif

#ifdef __cplusplus
}
#endif

#endif /* __L511_CORE_TUYA_CLOUD_TYPES_SHIM__ */
