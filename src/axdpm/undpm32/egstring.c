/*--------------------------------------------------------------------------
  egstring.c    string functions
--------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>
#include "egstring.h"

/*--------------------------------------------------------------------------
  定数宣言
--------------------------------------------------------------------------*/
#define STRMATCH_IGNORE_CASE  0x00000001  // アルファベットの大文字小文字を区別しない

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
static INT CharacterToInteger(UINT cNumChar);

static BOOL _StrMatch(LPCTSTR lpszString, LPCTSTR lpszPattern, DWORD dwCmpFlags);

/*--------------------------------------------------------------------------
  概要:         指定した文字をアルファベット大文字に変換します。

  宣言:         UINT ToUpperCase(
                    UINT cTestChar  // 変換する文字コード
                );

  パラメータ:   cTestChar
                    アルファベット大文字に変換したい文字コードを指定します。

  戻り値:       cTestChar をアルファベット大文字に変換した文字コードが返り
                ます。

  解説:         指定した文字をアルファベット大文字に変換します。
                アルファベット小文字以外の文字に変化はありません。
--------------------------------------------------------------------------*/
UINT ToUpperCase(UINT cTestChar)
{
  // アルファベット小文字
  if(cTestChar >= 'a' && cTestChar <= 'z')
  {
    // 大文字に変換する
    return cTestChar + ('A' - 'a');
  }
  else
  {
    // 何も変換しない
    return cTestChar;
  }
}

/*--------------------------------------------------------------------------
  概要:         指定した文字をアルファベット小文字に変換します。

  宣言:         UINT ToLowerCase(
                    UINT cTestChar  // 変換する文字コード
                );

  パラメータ:   cTestChar
                    アルファベット小文字に変換したい文字コードを指定します。

  戻り値:       cTestChar をアルファベット小文字に変換した文字コードが返り
                ます。

  解説:         指定した文字をアルファベット小文字に変換します。
                アルファベット大文字以外の文字に変化はありません。
--------------------------------------------------------------------------*/
UINT ToLowerCase(UINT cTestChar)
{
  // アルファベット大文字
  if(cTestChar >= 'A' && cTestChar <= 'Z')
  {
    // 小文字に変換する
    return cTestChar + ('a' - 'A');
  }
  else
  {
    // 何も変換しない
    return cTestChar;
  }
}

/*--------------------------------------------------------------------------
  概要:         文字列の次の文字コードを得ます。

  宣言:         UINT GetCharCode(
                    LPCTSTR lpszString  // 文字列へのポインタ
                );

  パラメータ:   lpszString
                    文字コードを得たい文字列のポインタを指定します。

  戻り値:       lpszString が示す文字の文字コードが返ります。

  解説:         指定した文字列の次の文字コードを得ます。
--------------------------------------------------------------------------*/
UINT GetCharCode(LPCTSTR lpszString)
{
#ifdef _MBCS
  // DBCS の先行バイトか
  if(IsDBCSLeadByte(*(LPBYTE)lpszString))
  {
    // ダブルバイト文字
    return (UINT)(*(LPWORD)lpszString);
  }
  else
  {
    // シングルバイト文字
    return (UINT)(*(LPBYTE)lpszString);
  }
#else
  // ワイド文字
  return *(LPWSTR)lpszString;
#endif // defined(_MBCS)
}

/*--------------------------------------------------------------------------
  概要:         文字列の次の文字の位置を取得します。

  宣言:         LPTSTR GetNextCharPointer(
                    LPCTSTR lpszString  // 文字列へのポインタ
                );

  パラメータ:   lpszString
                    次の文字位置を得たい文字列のポインタを指定します。

  戻り値:       lpszString が示す文字の次の文字へのポインタが返ります。

  解説:         指定した文字列の次の文字へのポインタを得ます。
                マルチバイト文字列を処理するのに便利です。
--------------------------------------------------------------------------*/
LPTSTR GetNextCharPointer(LPCTSTR lpszString)
{
#ifdef _MBCS
  // DBCS の先行バイトか
  if(IsDBCSLeadByte(*(LPBYTE)lpszString))
  {
    // ダブルバイト文字
    return (LPTSTR)((LPWORD)lpszString + 1);
  }
  else
  {
    // シングルバイト文字
    return (LPTSTR)((LPBYTE)lpszString + 1);
  }
#else
  // ワイド文字
  return (LPTSTR)((LPWSTR)lpszString + 1);
#endif // defined(_MBCS)
}

/*--------------------------------------------------------------------------
  概要:         文字列の数文字先の位置を取得します。

  宣言:         LPTSTR GetNextCharPointerN(
                    LPCTSTR lpszString  // 文字列へのポインタ
                    UINT cchMove        // 移動量 (文字数単位)
                );

  パラメータ:   lpszString
                    文字列のポインタを指定します。

                cchMove
                    進みたい文字数を指定します。

  戻り値:       lpszString の cchMove 文字先へのポインタが返ります。

  解説:         指定した文字列の数文字先へのポインタを得ます。
--------------------------------------------------------------------------*/
LPTSTR GetNextCharPointerN(LPCTSTR lpszString, UINT cchMove)
{
  return cchMove == 0 ? (LPTSTR)lpszString : GetNextCharPointerN(GetNextCharPointer(lpszString), cchMove - 1);
}

/*--------------------------------------------------------------------------
  概要:         文字列の空白部分を飛ばします。

  宣言:         LPTSTR SkipWhiteSpace(
                    LPCTSTR lpszString  // 文字列へのポインタ
                );

  パラメータ:   lpszString
                    空白部分を飛ばしたい文字列のポインタを指定します。

  戻り値:       空白以外の文字が現れた位置へのポインタを返します。

  解説:         文字列の空白部分を飛ばします。
--------------------------------------------------------------------------*/
LPTSTR SkipWhiteSpace(LPCTSTR lpszString)
{
  LPTSTR lpszNewPtr = (LPTSTR)lpszString; // 走査に使用するポインタ

  UINT cCurrent;  // 現在位置の文字コード

  // 1 文字取得
  cCurrent = GetCharCode(lpszNewPtr);

  // 空白文字ならば処理
  while(cCurrent == ' ' || cCurrent == '\t')
  {
    // 空白文字を飛ばす
    lpszNewPtr = GetNextCharPointer(lpszNewPtr);

    // 1 文字取得
    cCurrent = GetCharCode(lpszNewPtr);
  }
  return lpszNewPtr;
}

/*--------------------------------------------------------------------------
  概要:         文字を数値に変換します。

  宣言:         INT CharacterToInteger(
                    UINT cNumChar   // 数値に変換する文字
                );

  パラメータ:   cNumChar
                    数値に変換する文字を指定します。

  戻り値:       文字を数値に変換した値が返ります。
                不明な文字の場合は -1 が返ります。

  解説:         文字を数値に変換します。
--------------------------------------------------------------------------*/
static INT CharacterToInteger(UINT cNumChar)
{
  cNumChar = ToLowerCase(cNumChar);

  if(cNumChar >= '0' && cNumChar <= '9')
  {
    // 数字
    return cNumChar - '0';
  }
  else if(cNumChar >= 'a' && cNumChar <= 'z')
  {
    // アルファベット
    return cNumChar - 'a' + 10;
  }
  else
  {
    // その他
    return -1;
  }
}

/*--------------------------------------------------------------------------
  概要:         文字列を数値に変換します。

  宣言:         INT StringToInteger(
                    LPCTSTR lpszString, // 数値に変換する文字列
                    UINT nBase          // 基数
                );

  パラメータ:   lpszString
                    数値に変換する文字列へのポインタを指定します。

                nBase
                    数値の基数を指定します。

                    nBase に 0 を指定した場合、自動的に基数が選択されます。
                    詳しくは解説をご参照ください。

  戻り値:       文字列を数値に変換した値が返ります。

  解説:         文字列を数値に変換します。基数を指定することができます。

                nBase に 0 を指定した場合、自動的に基数が選択されます。
                0x で始まる文字列は 16 進数、それ以外の 0 で始まる文字列は
                8 進数、1 から 9 で始まる文字列は 10 進数、a から z で始ま
                る文字列は 11 から 36 までの値に割り当てます。
--------------------------------------------------------------------------*/
INT StringToInteger(LPCTSTR lpszString, UINT nBase)
{
  LPTSTR lpszCurrent;     // 走査位置
  INT iRet = 0;           // 返却される値
  BOOL bUnsigned = TRUE;  // 符号 (正: TRUE / 負: FALSE)
  UINT cCurrent;          // 走査位置の文字
  UINT nCharacter;        // 文字が表す数値

  // 空白文字列を飛ばす
  lpszCurrent = SkipWhiteSpace(lpszString);

  // 符号を調べる
  cCurrent = GetCharCode(lpszCurrent);
  switch(cCurrent)
  {
  case '-':
    bUnsigned = FALSE;
  case '+':
    lpszCurrent = GetNextCharPointer(lpszCurrent);
  }

  // 基数が指定されているか
  if(nBase == 0)
  {
    // 1 文字取得する
    nCharacter = CharacterToInteger(GetCharCode(lpszCurrent));

    if(nCharacter == 0)
    {
      // 次の文字を見て判断
      if(ToLowerCase(GetCharCode(GetNextCharPointer(lpszCurrent))) == 'x')
      {
        nBase = 16;
        lpszCurrent = GetNextCharPointerN(lpszCurrent, 2);
      }
      else{
        nBase = 8;
        lpszCurrent = GetNextCharPointer(lpszCurrent);
      }
    }
    if(nCharacter >= 1 && nCharacter <= 9)
    {
      // 数字 (0 以外)
      nBase = 10;
    }
    else if(nCharacter >= 10 && nCharacter <= 36)
    {
      // アルファベット
      nBase = nCharacter;
    }
  }

  // 1 文字取得する
  nCharacter = CharacterToInteger(GetCharCode(lpszCurrent));

  // 有効な文字か
  while(nCharacter != -1)
  {
    // 桁上がり
    iRet = iRet * nBase + nCharacter;

    // 1 文字進める
    lpszCurrent = GetNextCharPointer(lpszCurrent);

    // 1 文字取得する
    nCharacter = CharacterToInteger(GetCharCode(lpszCurrent));
  }

  // 符号反転
  if(!bUnsigned)
  {
    iRet = -iRet;
  }

  // 変換した値を返す
  return iRet;
}

/*--------------------------------------------------------------------------
  概要:         ワイルドカードを使用した文字列比較を行います。

  宣言:         BOOL _StrMatch(
                    LPCTSTR lpszString,     // 検索文字列へのポインタ
                    LPCTSTR lpszPattern,    // パターン文字列へのポインタ
                    DWORD dwCmpFlags        // 比較方法のオプション
                );

  パラメータ:   lpszString
                    比較に使われる、NULL で終わる文字列へのポインタを指定し
                    ます。

                lpszPattern
                    比較に使われる、NULL で終わる文字列へのポインタを指定し
                    ます。
                    lpszPattern にはワイルドカードを使用することができます。
                    '*' は任意の文字列、'?' は任意の 1 文字を表します。

                dwCmpFlags
                    2 つの文字列を比較する方法を表すオプションを指定します。

  戻り値:       2 つの文字列が一致した場合、0 以外の値が返ります。

                2 つの文字列が一致しなかった場合、0 が返ります。

  解説:         ワイルドカードを使用した文字列の比較を行います。
--------------------------------------------------------------------------*/
static BOOL _StrMatch(LPCTSTR lpszString, LPCTSTR lpszPattern, DWORD dwCmpFlags)
{
  UINT cString = GetCharCode(lpszString);     // 検索文字列の指す 1 文字
  UINT cPattern = GetCharCode(lpszPattern);   // パターン文字列の指す 1 文字

  // アルファベットの大文字小文字を区別しない
  if(dwCmpFlags & STRMATCH_IGNORE_CASE)
  {
    // 小文字に変換する
    cString = ToLowerCase(cString);
    cPattern = ToLowerCase(cPattern);
  }

  // 1 文字ずつ調べる
  switch(cPattern)
  {
  // NULL 文字
  case '\0':
    return cString == '\0';

  // 任意の文字列
  case '*':
    return _StrMatch(lpszString, GetNextCharPointer(lpszPattern), dwCmpFlags) || ((cString != '\0') && _StrMatch(GetNextCharPointer(lpszString), lpszPattern, dwCmpFlags));

  // 任意の 1 文字
  case '?':
    return (cString != '\0') && _StrMatch(GetNextCharPointer(lpszString), GetNextCharPointer(lpszPattern), dwCmpFlags);

  // それ以外の文字
  default:
    return (cPattern == cString) && _StrMatch(GetNextCharPointer(lpszString), GetNextCharPointer(lpszPattern), dwCmpFlags);
  }
}

/*--------------------------------------------------------------------------
  概要:         ワイルドカードを使用した文字列比較を行います。

  宣言:         BOOL StrMatch(
                    LPCTSTR lpszString, // 文字列へのポインタ
                    LPCTSTR lpszPattern // パターン文字列へのポインタ
                );

  パラメータ:   lpszString
                    比較に使われる、NULL で終わる文字列へのポインタを指定し
                    ます。

                lpszPattern
                    比較に使われる、NULL で終わる文字列へのポインタを指定し
                    ます。
                    lpszPattern にはワイルドカードを使用することができます。
                    '*' は任意の文字列、'?' は任意の 1 文字を表します。

  戻り値:       2 つの文字列が一致した場合、0 以外の値が返ります。

                2 つの文字列が一致しなかった場合、0 が返ります。

  解説:         ワイルドカードを使用した文字列の比較を行います。

                StrMatch 関数はアルファベットの大文字小文字を区別します。
                アルファベットの大文字小文字を区別しない場合は StrMatchI 関
                数を使用してください。
--------------------------------------------------------------------------*/
BOOL StrMatch(LPCTSTR lpszString, LPCTSTR lpszPattern)
{
  return _StrMatch(lpszString, lpszPattern, 0);
}

/*--------------------------------------------------------------------------
  概要:         ワイルドカードを使用した文字列比較を行います。

  宣言:         BOOL StrMatchI(
                    LPCTSTR lpszString, // 文字列へのポインタ
                    LPCTSTR lpszPattern // パターン文字列へのポインタ
                );

  パラメータ:   lpszString
                    比較に使われる、NULL で終わる文字列へのポインタを指定し
                    ます。

                lpszPattern
                    比較に使われる、NULL で終わる文字列へのポインタを指定し
                    ます。
                    lpszPattern にはワイルドカードを使用することができます。
                    '*' は任意の文字列、'?' は任意の 1 文字を表します。

  戻り値:       2 つの文字列が一致した場合、0 以外の値が返ります。

                2 つの文字列が一致しなかった場合、0 が返ります。

  解説:         ワイルドカードを使用した文字列の比較を行います。

                StrMatchI 関数はアルファベットの大文字小文字を区別しません。
                アルファベットの大文字小文字を区別する場合は StrMatch 関
                数を使用してください。
--------------------------------------------------------------------------*/
BOOL StrMatchI(LPCTSTR lpszString, LPCTSTR lpszPattern)
{
  return _StrMatch(lpszString, lpszPattern, STRMATCH_IGNORE_CASE);
}
