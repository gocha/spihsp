/*--------------------------------------------------------------------------
  egcnvstr.h    string converter between multibyte string and wide string
--------------------------------------------------------------------------*/

#ifndef __CNVSTR_H__
#define __CNVSTR_H__

#include <windows.h>

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
LPWSTR ConvertStringAToW(LPCSTR lpString);

LPSTR ConvertStringWToA(LPCWSTR lpString);

BOOL FreeConvertedString(LPVOID lpString);

/*--------------------------------------------------------------------------
  ファイル終了
--------------------------------------------------------------------------*/
#endif // !defined(__CNVSTR_H__)