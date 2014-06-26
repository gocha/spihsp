/*--------------------------------------------------------------------------
  egcnvstr.c    string converter between multibyte string and wide string
--------------------------------------------------------------------------*/

#include <windows.h>
#include "egalloc.h"
#include "egcnvstr.h"

/*--------------------------------------------------------------------------
  概要:         マルチバイト文字列をワイド文字列に変換します。

  宣言:         LPWSTR ConvertStringAToW(
                    LPCSTR lpszString   // マルチバイト文字列へのポインタ
                );

  パラメータ:   lpszString
                    ワイド文字列に変換するマルチバイト文字列へのポインタを
                    指定します。

  戻り値:       関数が成功すると、ワイド文字列へのポインタが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         マルチバイト文字列をワイド文字列に変換します。
                変換後の文字列は FreeConvertedString 関数で解放します。
--------------------------------------------------------------------------*/
LPWSTR ConvertStringAToW(LPCSTR lpszString)
{
  LPWSTR lpszStringW; // Unicode 文字列へのポインタ

  const UINT cchStr = lstrlenA(lpszString) + 1;   // 文字数 (NULL 文字含む)
  const UINT nSize = cchStr * sizeof(WCHAR);      // サイズ (バイト単位)

  // メモリを割り当てる
  lpszStringW = MemoryAlloc(nSize);

  if(lpszStringW)
  {
    // Unicode 文字列に変換
    MultiByteToWideChar(CP_ACP, 0, lpszString, cchStr, lpszStringW, cchStr);
  }

  // Unicode 文字列へのポインタを返す
  return lpszStringW;
}

/*--------------------------------------------------------------------------
  概要:         ワイド文字列をマルチバイト文字列に変換します。

  宣言:         LPSTR ConvertStringWToA(
                    LPCWSTR lpszString  // ワイド文字列へのポインタ
                );

  パラメータ:   lpszString
                    マルチバイト文字列に変換するワイド文字列へのポインタを
                    指定します。

  戻り値:       関数が成功すると、マルチバイト文字列へのポインタが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         ワイド文字列をマルチバイト文字列に変換します。
                変換後の文字列は FreeConvertedString 関数で解放します。
--------------------------------------------------------------------------*/
LPSTR ConvertStringWToA(LPCWSTR lpszString)
{
  LPSTR lpszStringA;  // ANSI 文字列へのポインタ

  const UINT cchStr = lstrlenW(lpszString) + 1;   // 文字数 (NULL 文字含む)
  const UINT nSize = cchStr * sizeof(CHAR);       // サイズ (バイト単位)

  // メモリを割り当てる
  lpszStringA = MemoryAlloc(nSize);

  if(lpszStringA)
  {
    // ANSI 文字列に変換
    WideCharToMultiByte(CP_ACP, 0, lpszString, cchStr, lpszStringA, cchStr, NULL, NULL);
  }

  // ANSI 文字列へのポインタを返す
  return lpszStringA;
}

/*--------------------------------------------------------------------------
  概要:         文字列バッファを解放します。

  宣言:         BOOL FreeConvertedString(
                    LPVOID lpszString   // メモリへのポインタ
                );

  パラメータ:   lpszString
                    解放したい文字列バッファへのポインタを指定します。
                    ConvertStringAToW 関数または ConvertStringWToA 関数が返
                    すポインタです。 

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         ConvertStringAToW 関数または ConvertStringWToA 関数が割り当
                てた文字列バッファを解放します。
--------------------------------------------------------------------------*/
BOOL FreeConvertedString(LPVOID lpszString)
{
  // メモリを解放する
  return MemoryFree(lpszString);
}
