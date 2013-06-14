// To enable the extension functions define SQLITE_ENABLE_EXTFUNC on compiling this module
#ifdef SQLITE_ENABLE_EXTFUNC
#define sqlite3_open    sqlite3_open_internal
#define sqlite3_open16  sqlite3_open16_internal
#define sqlite3_open_v2 sqlite3_open_v2_internal
#endif

#include "sqlite3.c"

#ifdef SQLITE_ENABLE_EXTFUNC
#undef sqlite3_open
#undef sqlite3_open16
#undef sqlite3_open_v2
#endif

#ifndef SQLITE_OMIT_DISKIO

#ifdef SQLITE_HAS_CODEC

/*
** Get the codec argument for this pager
*/

void* mySqlite3PagerGetCodec(
  Pager *pPager
){
#if (SQLITE_VERSION_NUMBER >= 3006016)
  return sqlite3PagerGetCodec(pPager);
#else
  return (pPager->xCodec) ? pPager->pCodecArg : NULL;
#endif
}

/*
** Set the codec argument for this pager
*/

void mySqlite3PagerSetCodec(
  Pager *pPager,
  void *(*xCodec)(void*,void*,Pgno,int),
  void (*xCodecSizeChng)(void*,int,int),
  void (*xCodecFree)(void*),
  void *pCodec
){
  sqlite3PagerSetCodec(pPager, xCodec, xCodecSizeChng, xCodecFree, pCodec);
}

#include "rijndael.c"
#include "codec.c"
#include "codecext.c"

#endif

#endif

#ifdef SQLITE_ENABLE_EXTFUNC

#include "extensionfunctions.c"

SQLITE_API int sqlite3_open(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
)
{
  int ret = sqlite3_open_internal(filename, ppDb);
  if (ret == 0)
  {
    RegisterExtensionFunctions(*ppDb);
  }
  return ret;
}

SQLITE_API int sqlite3_open16(
  const void *filename,   /* Database filename (UTF-16) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
)
{
  int ret = sqlite3_open16_internal(filename, ppDb);
  if (ret == 0)
  {
    RegisterExtensionFunctions(*ppDb);
  }
  return ret;
}

SQLITE_API int sqlite3_open_v2(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
)
{
  int ret = sqlite3_open_v2_internal(filename, ppDb, flags, zVfs);
  if (ret == 0)
  {
    RegisterExtensionFunctions(*ppDb);
  }
  return ret;
}

#endif
