/* $Id: portdefs.h,v 1.21 2002/11/06 07:41:23 cmatsuoka Exp $ */

/*
 * This file contains port-specific definitions. Port makefiles may
 * set a different include path to override definitions in sarien.h
 */


/*
 * Features
 */
//#define FAKE_PALMOS
#  undef USE_HIRES
#  undef USE_PCM_SOUND
#  undef USE_MOUSE
#  define  PCCGA
#  undef USE_CONSOLE
#  undef USE_HIRES
# define USE_COMMAND_LINE
#  undef AGDS_SUPPORT
#  undef OPT_LIST_OBJECTS
#  undef OPT_PICTURE_VIEWER
#  undef OPT_LIST_DICT
#  undef HAVE_VSNPRINTF
#  undef _TRACE




#ifdef DREAMCAST
#  define DC_BASE_PATH		"/cd"
#  define DC_GFX_PATH		"/cd/gfx"
#  define VMU_PATH		"/vmu/%s/SDC-%s-%d"
#  define UNKNOWN_GAME		"Unknown"
#  undef USE_COMMAND_LINE
char g_gamename[255];
static char g_vmu_port[2];
#endif

#ifdef __TURBOC__
#  undef USE_PCM_SOUND
#  undef USE_MOUSE
#endif

#ifdef PCCGA
#  undef USE_HIRES
#endif

#  include <stdlib.h>
#  include <assert.h>
   typedef signed int Err;
//#define _TRACE

#ifdef FAKE_PALMOS
#  undef USE_CONSOLE
#  undef USE_PCM_SOUND
#  undef USE_HIRES
#  undef USE_COMMAND_LINE
#  undef AGDS_SUPPORT
#  undef OPT_LIST_OBJECTS
#  undef OPT_PICTURE_VIEWER
#  undef OPT_LIST_DICT
#endif

/*
 * Memory allocation
 */

/*
 * From the Turbo C FAQ:
 *
 * Q. I have a working program that dynamically allocates memory
 *    using malloc() or calloc() in small data models (tiny, small,
 *    and medium). When I compile this program in large data models
 *    (compact, large, and huge), my program hangs.
 * A. Make sure that you have #include <alloc.h> in your program.
 */
#ifdef __TURBOC__
#  include <alloc.h>
#endif


/* Environment variable containing the path name for the users's
 * private files ($HOME in Unix, %USERPROFILE% in Win32)
 * DATADIR conflicts with ObjIdl.h in win32 SDK, renamed to DATA_DIR
 */
#if defined (WIN32) || defined (__MSDOS__)
#  define HOMEDIR "USERPROFILE"
#  define DATA_DIR "Sarien"
#else
#  define HOMEDIR "HOME"
#  define DATA_DIR ".sarien"
#endif


/*
 * Data types
 */
#if 0
   typedef unsigned char	UINT8;
   typedef signed char		SINT8;
   typedef unsigned short	UINT16;
   typedef signed short		SINT16;
   typedef unsigned int		UINT32;
   typedef signed int		SINT32;
#else
#if defined(PALMOS)
   typedef UInt8		UINT8;
   typedef UInt16		UINT16;
   typedef UInt32		UINT32;
   typedef Int8			SINT8;
   typedef Int16		SINT16;
   typedef Int32		SINT32;
#elif defined(__QDOS__)
   typedef unsigned char	UINT8;
   typedef signed char		SINT8;
   typedef unsigned short	UINT16;
   typedef signed short		SINT16;
   typedef unsigned int		UINT32;
   typedef signed int		SINT32;
#undef __MSDOS__
#elif defined(__MSDOS__)
   typedef unsigned char	UINT8;
   typedef signed char		SINT8;
   typedef unsigned short	UINT16;
   typedef signed short		SINT16;
   typedef unsigned long	UINT32;
   typedef signed long		SINT32;
#elif defined(WIN32)
   typedef unsigned char	UINT8;
   typedef signed char		SINT8;
   typedef unsigned short	UINT16;
   typedef signed short		SINT16;
#  ifndef _BASETSD_H
     typedef unsigned int	UINT32;
#  endif
   typedef signed int		SINT32;
#else
   typedef unsigned char	UINT8;
   typedef signed char		SINT8;
   typedef unsigned short	UINT16;
   typedef signed short		SINT16;
   typedef unsigned int		UINT32;
   typedef signed int		SINT32;
#endif
#endif

/*
 * Version and other definitions
 */
#ifdef NATIVE_WIN32
#  define INLINE __inline
#  define VERSION __TIMESTAMP__
#endif

