/*--------------------------------------------------------------------------
  crypt.c       UNDPM32.DLL crypt main
--------------------------------------------------------------------------*/

#include <windows.h>
#include <stdlib.h>

#include "undpm32.h"
#include "main.h"
#include "info.h"
#include "crypt.h"

/*--------------------------------------------------------------------------
  定数宣言
--------------------------------------------------------------------------*/
#ifdef HSP33_SUPPORT
#define CRYPT_DPM_PRM1          (BYTE)0x5a
#define CRYPT_DPM_PRM2          (BYTE)0xa5
#else
#define CRYPT_DPM_PRM1          (BYTE)0x55
#define CRYPT_DPM_PRM2          (BYTE)0xaa
#endif

#define CRYPT_EXE_PRM1          3
#define CRYPT_EXE_PRM2          5
#define CRYPT_EXE_PRM3          (BYTE)0xaa

#define CRYPT_EXE_KEY_DEFAULT   MAKEWORD(0xaa, 0x55)

#define CRYPT_CRYPT_KEY_VER255  MAKEWORD(0x55, 0x01)

/*--------------------------------------------------------------------------
  概要:         暗号化されたバッファを復号化します。

  宣言:         LPVOID UnDpmDecrypt(
                    LPVOID lpDest,      // コピー先バッファ
                    LPCVOID lpSrc,      // コピー元バッファ
                    DWORD dwSize,       // コピーするサイズ
                    DWORD dwDpmFileKey, // 暗号鍵
                    BYTE byDecodeSeed   // 復号バイト初期値
                );

  パラメータ:   lpDest
                    コピー先バッファへのポインタを指定します。

                lpSrc
                    コピー元バッファへのポインタを指定します。

                dwSize
                    コピーするサイズを指定します。

                dwDpmFileKey
                    暗号鍵を指定します。

                byDecodeSeed
                    復号バイトの初期値を指定します。
                    過去のバージョンでは必ず0でしたが、
                    HSP3.3以降では、前の格納ファイルに依存します。

  戻り値:       lpDest が返ります。

  解説:         暗号化されたバッファを復号化します。
                外部 DPM ファイル以外のものはそのままでは復号化できません。
                予め dwDpmFileKey を算出しておくことで復号化できます。
--------------------------------------------------------------------------*/
LPVOID UnDpmDecrypt(LPVOID lpDest, LPCVOID lpSrc, DWORD dwSize, DWORD dwDpmFileKey, BYTE byDecodeSeed)
{
  const WORD wCryptKey = UnDpmMakeCryptKey(UnDpmMakeDpmKey(dwDpmFileKey), 0, FALSE);
  const BYTE byCryptKey1 = LOBYTE(wCryptKey);
  const BYTE byCryptKey2 = HIBYTE(wCryptKey);

  DWORD i;                      // 制御変数
  BYTE byData = byDecodeSeed;   // 書き込みデータ

  // 暗号化されているか
  if(dwDpmFileKey == CRYPT_DPMFILE_KEY_NOCRYPT)
  {
    // コピー
    if (lpDest != lpSrc)
    {
      memcpy(lpDest, lpSrc, dwSize);
    }
  }
  else
  {
    // 復号化
    for(i = 0; i < dwSize; i++)
    {
#ifdef HSP33_SUPPORT
      byData += (BYTE)((*((LPBYTE)lpSrc + i) ^ byCryptKey1) - byCryptKey2);
#else
      byData += (BYTE)((*((LPBYTE)lpSrc + i) - byCryptKey2) ^ byCryptKey1);
#endif
      *((LPBYTE)lpDest + i) = byData;
    }
  }

  return lpDest;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブの暗号鍵を調整します。

  宣言:         BOOL UnDpmAdjustCryptKey(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         アーカイブの暗号鍵を調整します。

                古いバージョンのアーカイブや実行ファイル形式のアーカイブを
                開くことができるようになります。
--------------------------------------------------------------------------*/
BOOL UnDpmAdjustCryptKey(HDPM hDpm)
{
  LPDPMINTERNALINFO lpInternalInfo = UnDpmGetInternalInfo(hDpm);
  LPDPMHED lpDpmHed = (LPDPMHED)(lpInternalInfo + 1);
  LPDPMFILEDIR lpFirstFile = (LPDPMFILEDIR)(lpDpmHed + 1);

  DWORD i;    // 制御変数

  // 暗号化のないバージョンは処理しない
  if(lpInternalInfo->dwVersion > DPM_VERSION_HSP2X)
  {
    // すべてのファイルを処理
    for(i = 0; i < lpDpmHed->nNumberOfFile; i++)
    {
      // 暗号化されていない
      if((lpFirstFile + i)->dwCryptKey == CRYPT_DPMFILE_KEY_NOCRYPT)
      {
        continue;
      }

      // バージョンによって分岐
      switch(lpInternalInfo->dwVersion)
      {
      case DPM_VERSION_HSP255:
        (lpFirstFile + i)->dwCryptKey = UnDpmMakeDpmFileKey(CRYPT_CRYPT_KEY_VER255);
        break;

      case DPM_VERSION_HSP26:
        // 実行ファイル形式
        if(lpInternalInfo->bExecutable)
        {
          (lpFirstFile + i)->dwCryptKey = UnDpmMakeDpmFileKey(
            UnDpmMakeCryptKey(
              UnDpmMakeDpmKey((lpFirstFile + i)->dwCryptKey), 
              UnDpmMakeExeKey(lpInternalInfo->dwExeFileKey, lpInternalInfo->dwFileSize - lpDpmHed->dwOffsetToDS), 
              lpInternalInfo->bExecutable
            )
          );
        }
        break;
      }
    }
  }
  return TRUE;
}

/*--------------------------------------------------------------------------
  概要:         DPM ファイルの鍵から DPM 鍵を生成します。

  宣言:         WORD UnDpmMakeDpmKey(
                    DWORD dwDpmFileKey  // DPM ファイルの鍵
                );

  パラメータ:   dwDpmFileKey
                    DPM ファイルの鍵を指定します。

  戻り値:       生成された鍵を返します。

  解説:         DPM ファイルの鍵から DPM 鍵を生成します。
--------------------------------------------------------------------------*/
WORD UnDpmMakeDpmKey(DWORD dwDpmFileKey)
{
  const BYTE byDpmKey1 = (BYTE)((LOBYTE(LOWORD(dwDpmFileKey)) + CRYPT_DPM_PRM1) ^ LOBYTE(HIWORD(dwDpmFileKey)));
  const BYTE byDpmKey2 = (BYTE)((HIBYTE(LOWORD(dwDpmFileKey)) + CRYPT_DPM_PRM2) ^ HIBYTE(HIWORD(dwDpmFileKey)));
  return MAKEWORD(byDpmKey1, byDpmKey2);
}

/*--------------------------------------------------------------------------
  概要:         実行ファイルファイルの鍵から実行ファイル鍵を生成します。

  宣言:         WORD UnDpmMakeExeKey(
                    DWORD dwExeFileKey,     // 実行ファイルの鍵
                    DWORD dwDataSectionSize // データセクションのサイズ
                );

  パラメータ:   dwExeFileKey
                    実行ファイルの鍵を指定します。

                dwDataSectionSize
                    DPM ファイルのデータセクションサイズを指定します。

  戻り値:       生成された鍵を返します。

  解説:         実行ファイルファイルの鍵から実行ファイル鍵を生成します。
--------------------------------------------------------------------------*/
WORD UnDpmMakeExeKey(DWORD dwExeFileKey, DWORD dwDataSectionSize)
{
  const BYTE byExeKey1 = (BYTE)(((UINT)LOBYTE(LOWORD(dwExeFileKey)) * (UINT)LOBYTE(HIWORD(dwExeFileKey)) / CRYPT_EXE_PRM1) ^ dwDataSectionSize);
  const BYTE byExeKey2 = (BYTE)(((UINT)HIBYTE(LOWORD(dwExeFileKey)) * (UINT)HIBYTE(HIWORD(dwExeFileKey)) / CRYPT_EXE_PRM2) ^ dwDataSectionSize ^ CRYPT_EXE_PRM3);
  return MAKEWORD(byExeKey1, byExeKey2);
}

/*--------------------------------------------------------------------------
  概要:         暗号鍵を生成します。

  宣言:         WORD UnDpmMakeCryptKey(
                    WORD wDpmKey,   // DPM ファイルから生成される鍵
                    WORD wExeKey,   // 実行ファイルから生成される鍵
                    BOOL bUseExeKey // 実行ファイルの鍵を使うか
                );

  パラメータ:   wDpmKey
                    DPM ファイルから生成される鍵

                wCryptKey
                    実行ファイルから生成される鍵を指定します。

                bUseExeKey
                    実行ファイルから生成される鍵を使うかどうかを指定します。

  戻り値:       生成された暗号鍵を返します。

  解説:         暗号鍵を生成します。
--------------------------------------------------------------------------*/
WORD UnDpmMakeCryptKey(WORD wDpmKey, WORD wExeKey, BOOL bUseExeKey)
{
  BYTE byKey1;    // 暗号鍵 (下位)
  BYTE byKey2;    // 暗号鍵 (上位)

  // 実行ファイルの鍵を使うか
  if(!bUseExeKey)
  {
    // デフォルトの値を使う
    wExeKey = CRYPT_EXE_KEY_DEFAULT;
  }

  // 鍵を生成する
  byKey1 = (BYTE)(LOBYTE(wDpmKey) + LOBYTE(wExeKey));
  byKey2 = (BYTE)(HIBYTE(wDpmKey) + HIBYTE(wExeKey));

  // 生成した鍵を返す
  return MAKEWORD(byKey1, byKey2);
}

/*--------------------------------------------------------------------------
  概要:         DPM ファイルの鍵を生成します。

  宣言:         DWORD UnDpmMakeDpmFileKey(
                    WORD wCryptKey  // 暗号鍵
                );

  パラメータ:   wCryptKey
                    暗号鍵を指定します。

  戻り値:       生成された DPM ファイルの鍵を返します。

  解説:         DPM ファイルの鍵を生成します。
--------------------------------------------------------------------------*/
DWORD UnDpmMakeDpmFileKey(WORD wCryptKey)
{
  const BYTE byDpmFileKey1 = (BYTE)0; // 好きな値で構いません
  const BYTE byDpmFileKey2 = (BYTE)0; // これらを元に残りを生成します
  const BYTE byDpmFileKey3 = (BYTE)((LOBYTE(wCryptKey) - LOBYTE(CRYPT_EXE_KEY_DEFAULT)) ^ (byDpmFileKey1 + CRYPT_DPM_PRM1));
  const BYTE byDpmFileKey4 = (BYTE)((HIBYTE(wCryptKey) - HIBYTE(CRYPT_EXE_KEY_DEFAULT)) ^ (byDpmFileKey2 + CRYPT_DPM_PRM2));
  return MAKELONG(MAKEWORD(byDpmFileKey1, byDpmFileKey2), MAKEWORD(byDpmFileKey3, byDpmFileKey4));
}
