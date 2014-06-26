/*--------------------------------------------------------------------------
  egstring.h    string functions
--------------------------------------------------------------------------*/

#ifndef __EGSTRING_H__
#define __EGSTRING_H__

#include <windows.h>
#include <tchar.h>

/*--------------------------------------------------------------------------
  マクロ定義
--------------------------------------------------------------------------*/
#ifndef countof
#define countof(array)  (sizeof(array) / sizeof(array[0]))
#endif // !defined(countof)

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
UINT ToUpperCase(UINT cTestChar);
UINT ToLowerCase(UINT cTestChar);

UINT GetCharCode(LPCTSTR lpszString);

LPTSTR GetNextCharPointer(LPCTSTR lpszString);
LPTSTR GetNextCharPointerN(LPCTSTR lpszString, UINT cchMove);

LPTSTR SkipWhiteSpace(LPCTSTR lpszString);

INT StringToInteger(LPCTSTR lpszString, UINT nBase);

BOOL StrMatch(LPCTSTR lpszString, LPCTSTR lpszPattern);
BOOL StrMatchI(LPCTSTR lpszString, LPCTSTR lpszPattern);

/*--------------------------------------------------------------------------
  ファイル終了
--------------------------------------------------------------------------*/
#endif // !defined(__EGSTRING_H__)