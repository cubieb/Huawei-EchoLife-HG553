/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/

//
// bcmtypes.h - misc useful typedefs
//
#ifndef BCMTYPES_H
#define BCMTYPES_H

// These are also defined in typedefs.h in the application area, so I need to
// protect against re-definition.

#ifndef _TYPEDEFS_H_

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
typedef unsigned long long  uint64;
typedef signed char     int8;
typedef signed short    int16;
typedef signed long     int32;
typedef signed long long    int64;

#if !defined(__cplusplus) && (!defined(__KERNEL__) || !defined(_LINUX_TYPES_H))
typedef	int	bool;
#endif

#endif

typedef unsigned char   byte;
// typedef unsigned long   sem_t;

typedef unsigned long   HANDLE,*PULONG,DWORD,*PDWORD;
typedef signed long     LONG;
typedef signed long    *PLONG;

typedef unsigned int    *PUINT;
typedef signed int      INT;

typedef unsigned short  *PUSHORT;
typedef signed short *PSHORT;
typedef unsigned short  WORD,*PWORD;

typedef unsigned char   *PUCHAR;
typedef signed char     *PCHAR;

typedef void            *PVOID;

typedef unsigned char   BOOLEAN, *PBOOL, *PBOOLEAN;

typedef unsigned char   BYTE,*PBYTE;

typedef signed int      *PINT;

typedef signed char     INT8;
typedef signed short    INT16;
typedef signed long     INT32;

#ifndef _UINT8_
#define _UINT8_
typedef unsigned char   UINT8;
#endif
#ifndef _UINT16_
#define _UINT16_
typedef unsigned short  UINT16;
#endif
#ifndef _UINT32_
#define _UINT32_
typedef unsigned long   UINT32;
#endif
typedef unsigned int    UINT;

#if 1
#ifndef SHORT
typedef signed short    SHORT;
#endif /* SHORT */
#ifndef UCHAR
typedef unsigned char   UCHAR;
#endif /* UCHAR */
#ifndef USHORT
typedef unsigned short  USHORT;
#endif /* USHORT */
#ifndef ULONG
typedef unsigned long   ULONG;
#endif /* ULONG */
#ifndef VOID
typedef void            VOID;
#endif /* VOID */
#ifndef BOOL
typedef unsigned char   BOOL;
#endif /* BOOL */
#endif

// These are also defined in typedefs.h in the application area, so I need to
// protect against re-definition.
#ifndef TYPEDEFS_H

// Maximum and minimum values for a signed 16 bit integer.
#define MAX_INT16 32767
#define MIN_INT16 -32768

// Useful for true/false return values.  This uses the
// Taligent notation (k for constant).
typedef enum
{
    kFalse = 0,
    kTrue = 1
} Bool;

#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#define READ32(addr)        (*(volatile UINT32 *)((ULONG)&addr))
#define READ16(addr)        (*(volatile UINT16 *)((ULONG)&addr))
#define READ8(addr)         (*(volatile UINT8  *)((ULONG)&addr))

#define VIRT_TO_PHY(a)  (((unsigned long)(a)) & 0x1fffffff)

#endif
