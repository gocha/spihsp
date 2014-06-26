/*--------------------------------------------------------------------------
  egalloc.h     memory allocation functions for Windows
--------------------------------------------------------------------------*/

#ifndef __EGALLOC_H__
#define __EGALLOC_H__

#include <windows.h>

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
LPVOID MemoryAlloc(SIZE_T nBytes);

LPVOID MemoryReAlloc(LPVOID lpMem, SIZE_T nBytes);

DWORD MemorySize(LPCVOID lpMem);

BOOL MemoryFree(LPVOID lpMem);

/*--------------------------------------------------------------------------
  ファイル終了
--------------------------------------------------------------------------*/
#endif // !defined(__EGALLOC_H__)