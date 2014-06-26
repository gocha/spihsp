/*--------------------------------------------------------------------------
  main.c        UNDPM32.DLL main
--------------------------------------------------------------------------*/

#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "undpm32.h"
#include "main.h"
#include "info.h"
#include "crypt.h"

#include "egalloc.h"
#include "egcnvstr.h"
#include "egstring.h"

/*--------------------------------------------------------------------------
  定数宣言
--------------------------------------------------------------------------*/
#define DPM_SIGNATURE       "DPMX"          // DPM のシグネチャ

/*--------------------------------------------------------------------------
  マクロ定義
--------------------------------------------------------------------------*/
#define DWORDAlignment(x)   ((x) + 3 & ~3)  // DWORD 境界に揃える

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
static LPVOID   UnDpmGetDpmByHandle(HDPM hDpm);

static LPDPMFILEDIR UnDpmGetFileDirPtr(HDPM hDpm, DWORD iFileId);

static HDPM     UnDpmOpenDpmMem(LPVOID lpBuffer, DWORD dwSize);
static HDPM     UnDpmOpenExeMem(LPVOID lpBuffer, DWORD dwSize);
static BOOL     UnDpmCheckDpmMem(LPVOID lpBuffer, DWORD dwSize, LPDWORD lpdwArcSize);
static BOOL     UnDpmCheckExeMem(LPVOID lpBuffer, DWORD dwSize, LPHSPHED *lpHspHed, LPDPMHED *lpDpmHed, LPDWORD lpdwArcSize);
static BOOL     UnDpmSearchHspHed(LPVOID lpBuffer, DWORD dwSize, LPHSPHED *lpHspHed, LPDPMHED *lpDpmHed, LPDWORD lpdwArcSize);
static DWORD    UnDpmCheckArcVersion(HDPM hDpm);

/*--------------------------------------------------------------------------
  概要:         アーカイブを閉じます。

  宣言:         BOOL UnDpmCloseArchive(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         アーカイブを閉じます。UnDpmOpenArchive 関数などで取得した
                ハンドルを渡します。解放したハンドルは使用しないでください。
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmCloseArchive(HDPM hDpm)
{
  return MemoryFree(hDpm);
}

/*--------------------------------------------------------------------------
  概要:         アーカイブを開きます。 (ANSI)

  宣言:         HDPM UnDpmOpenArchiveA(
                    LPCSTR lpszFileName // ファイル名
                );

  パラメータ:   lpszFileName
                    開きたいアーカイブのファイル名を指定します。
                dwOffset
                    アーカイブの先頭位置を指定します。通常は0を指定します。

  戻り値:       関数が成功すると、アーカイブのハンドルが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         アーカイブを開きます。UnDpmOpenArchive 関数で渡されるハンド
                ルを各種関数に渡すことで操作を行います。
--------------------------------------------------------------------------*/
UNDPMAPI HDPM UnDpmOpenArchiveA(LPCSTR lpszFileName, DWORD dwOffset)
{
  HDPM hDpm;              // アーカイブハンドル
  LPWSTR lpszFileNameW;   // Unicode に変換したファイル名

  // 文字列を Unicode に変換
  lpszFileNameW = ConvertStringAToW(lpszFileName);

  // Unicode 版の関数を呼び出し
  hDpm = UnDpmOpenArchiveW(lpszFileNameW, dwOffset);

  // Unicode に変換した文字列を解放
  FreeConvertedString(lpszFileNameW);

  // アーカイブハンドルを返す
  return hDpm;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブを開きます。 (Unicode)

  宣言:         HDPM UnDpmOpenArchiveW(
                    LPCWSTR lpszFileName    // ファイル名
                );

  パラメータ:   lpszFileName
                    開きたいアーカイブのファイル名を指定します。
                dwOffset
                    アーカイブの先頭位置を指定します。通常は0を指定します。

  戻り値:       関数が成功すると、アーカイブのハンドルが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         アーカイブの操作を行うためのハンドルを取得します。
                このハンドルを使ってアーカイブに操作を行います。
--------------------------------------------------------------------------*/
UNDPMAPI HDPM UnDpmOpenArchiveW(LPCWSTR lpszFileName, DWORD dwOffset)
{
  HDPM hDpm = NULL;       // アーカイブハンドル

  LPVOID lpBuffer;        // ファイルバッファ
  HANDLE hFile;           // ファイルハンドル
  DWORD dwFileSize;       // ファイルサイズ
  DWORD dwReadSize;       // ファイルから読み込んだサイズ

  // ファイルを開く
  hFile = CreateFileW(
    lpszFileName, 
    GENERIC_READ, 
    0, 
    NULL, 
    OPEN_EXISTING, 
    FILE_ATTRIBUTE_NORMAL, 
    NULL
  );

  // ファイルが開けたか
  if(hFile != INVALID_HANDLE_VALUE)
  {
    // ファイルサイズを取得
    dwFileSize = GetFileSize(hFile, NULL);

    // 正常にサイズを取得できたか
    if(dwFileSize != -1)
    {
      // 先頭とする位置を移動させる
      if(SetFilePointer(hFile, dwOffset, NULL, FILE_BEGIN) == dwOffset)
      {
        // ファイルサイズを調整
        dwFileSize -= dwOffset;

        // メモリブロックの割り当て
        lpBuffer = MemoryAlloc(dwFileSize);

        // 正常にメモリが割り当てられたか
        if(lpBuffer)
        {
          // ファイルを読み込む
          ReadFile(hFile, lpBuffer, dwFileSize, &dwReadSize, NULL);

          // 正常に読み込みできたか
          if(dwFileSize == dwReadSize)
          {
            // メモリからアーカイブを開く
            hDpm = UnDpmOpenArchiveMem(lpBuffer, dwFileSize);
          }

          // メモリを解放する
          MemoryFree(lpBuffer);
        }
      }
    }

    // ファイルを閉じる
    CloseHandle(hFile);
  }
  return hDpm;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブをメモリから開きます。

  宣言:         HDPM UnDpmOpenArchiveMem(
                    LPVOID lpBuffer,    // バッファへのポインタ
                    DWORD dwSize        // アーカイブのサイズ (バイト単位)
                );

  パラメータ:   lpBuffer
                    アーカイブを保持しているバッファへのポインタを指定しま
                    す。

                dwSize
                    アーカイブのサイズをバイト単位で指定します。
                    実際のサイズより大きい値を指定しても構いません。

  戻り値:       関数が成功すると、アーカイブのハンドルが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         アーカイブの操作を行うためのハンドルを取得します。
                このハンドルを使ってアーカイブに操作を行います。
--------------------------------------------------------------------------*/
UNDPMAPI HDPM UnDpmOpenArchiveMem(LPVOID lpBuffer, DWORD dwSize)
{
  HDPM hDpm = NULL;   // アーカイブハンドル

  // 実行ファイルを開く
  hDpm = UnDpmOpenExeMem(lpBuffer, dwSize);

  // エラーが発生した
  if(!hDpm)
  {
    // DPM アーカイブを開く
    hDpm = UnDpmOpenDpmMem(lpBuffer, dwSize);
  }

  // 正常に読み込みできた
  if(hDpm)
  {
    // 暗号鍵を調整
    UnDpmAdjustCryptKey(hDpm);
#if 0
    // アーカイブ読込時に全体を復号する
    {
      BYTE byDecodeSeed = 0;
      DWORD fileCount = UnDpmGetFileCount(hDpm);
      DWORD iFileId;
      for (iFileId = 1; iFileId <= fileCount; iFileId++)
      {
      	LPDPMFILEDIR lpFileDir; // ファイル情報
      	LPBYTE lpFileData;  // ファイルデータ部
        DWORD dwFileSize;   // ファイルサイズ

        // ファイル情報を取得
        lpFileDir = UnDpmGetFileDirPtr(hDpm, iFileId);

        // ファイル全体を復号
        if(lpFileDir->dwCryptKey != CRYPT_DPMFILE_KEY_NOCRYPT)
        {
          dwFileSize = UnDpmGetOriginalSize(hDpm, iFileId);
          if(dwFileSize > 0)
          {
            lpFileData = (LPBYTE)UnDpmGetDataSection(hDpm) + lpFileDir->dwOffsetInDS;
            UnDpmDecrypt(lpFileData, lpFileData, dwFileSize, lpFileDir->dwCryptKey, byDecodeSeed);
          }

          // 暗号鍵を復号済みとしてマーク
          lpFileDir->dwCryptKey = CRYPT_DPMFILE_KEY_NOCRYPT;
        }
      }
    }
#endif
  }

  // アーカイブハンドルを返す
  return hDpm;
}

/*--------------------------------------------------------------------------
  概要:         ファイルを解凍します。 (ANSI)

  宣言:         BOOL UnDpmExtractA(
                    HDPM hDpm,          // アーカイブハンドル
                    DWORD iFileId,      // ファイル ID
                    LPSTR lpszDirName   // 解凍先ディレクトリ
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

                szDirName
                    解凍先のディレクトリを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         ファイルを解凍します。
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmExtractA(HDPM hDpm, DWORD iFileId, LPSTR lpszDirName)
{
  BOOL bRet;              // 解凍結果
  LPWSTR lpszDirNameW;    // 解凍先ディレクトリ (Unicode)

  // 文字列を Unicode に変換
  lpszDirNameW = ConvertStringAToW(lpszDirName);

  // Unicode 版の関数を呼び出し
  bRet = UnDpmExtractW(hDpm, iFileId, lpszDirNameW);

  // Unicode に変換した文字列を解放
  FreeConvertedString(lpszDirNameW);

  // 結果を返す
  return bRet;
}

/*--------------------------------------------------------------------------
  概要:         ファイルを解凍します。 (Unicode)

  宣言:         BOOL UnDpmExtractW(
                    HDPM hDpm,          // アーカイブハンドル
                    DWORD iFileId,      // ファイル ID
                    LPSTR lpszDirName   // 解凍先ディレクトリ
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

                szDirName
                    解凍先のディレクトリを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         ファイルを解凍します。
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmExtractW(HDPM hDpm, DWORD iFileId, LPWSTR lpszDirName)
{
  BOOL bRet = FALSE;              // 処理の結果
  DWORD dwOriginalSize;           // 解凍後のサイズ
  DWORD dwWrittenSize;            // 書き込みされたサイズ
  WCHAR szCurrentDir[_MAX_PATH];  // カレントディレクトリ
  WCHAR szFileName[_MAX_PATH];    // 書き込みファイル名
  BOOL bMoveDirectory;            // ディレクトリ移動フラグ
  LPVOID lpBuffer;                // 書き込みバッファ
  HANDLE hFile;                   // ファイルハンドル

  // ディレクトリ移動の必要性があるか
  bMoveDirectory = (BOOL)((lpszDirName != NULL) && (StrCmpW(lpszDirName, L"") != 0));

  // 正常なファイル ID か
  if(UnDpmIsFileId(hDpm, iFileId))
  {
    // 解凍後のサイズを取得
    dwOriginalSize = UnDpmGetOriginalSize(hDpm, iFileId);

    // メモリバッファの割り当て
    lpBuffer = MemoryAlloc(dwOriginalSize);

    // 正常にメモリが割り当てられたか
    if(lpBuffer)
    {
      // メモリバッファに解凍
      if(UnDpmExtractMem(hDpm, iFileId, lpBuffer, dwOriginalSize))
      {
        // ファイル名を取得
        UnDpmGetFileNameW(hDpm, iFileId, szFileName, countof(szFileName));

        // ディレクトリ移動の必要があるか
        if(bMoveDirectory)
        {
          // カレントディレクトリを取得
          GetCurrentDirectoryW(countof(szCurrentDir), szCurrentDir);

          // カレントディレクトリを移動
          SetCurrentDirectoryW(lpszDirName);
        }

        // ファイルを作成
        hFile = CreateFileW(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // ファイル作成に成功したか
        if(hFile != INVALID_HANDLE_VALUE)
        {
          // データを書き込み
          WriteFile(hFile, lpBuffer, dwOriginalSize, &dwWrittenSize, NULL);

          // 正常に書き込みできたか
          if(dwWrittenSize == dwOriginalSize)
          {
            bRet = TRUE;
          }

          // ファイルを閉じる
          CloseHandle(hFile);
        }

        // ディレクトリ移動の必要があるか
        if(bMoveDirectory)
        {
          // カレントディレクトリを戻す
          SetCurrentDirectoryW(szCurrentDir);
        }
      }

      // メモリバッファを解放
      MemoryFree(lpBuffer);
    }
  }

  return bRet;
}

/*--------------------------------------------------------------------------
  概要:         メモリバッファにファイルを解凍します。

  宣言:         BOOL UnDpmExtractMem(
                    HDPM hDpm,          // アーカイブハンドル
                    DWORD iFileId,      // ファイル ID
                    LPVOID lpBuffer,    // データを格納するバッファ
                    DWORD dwSize        // 読み取りサイズ
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

                lpBuffer
                    解凍先のメモリバッファへのポインタを指定します。

                dwSize
                    解凍を行うサイズを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         メモリバッファにファイルを解凍します。
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmExtractMem(HDPM hDpm, DWORD iFileId, LPVOID lpBuffer, DWORD dwSize)
{
  DPMFILEDIR FileDir;     // ファイル情報

  // ファイル情報を取得
  if(!UnDpmGetFileDir(hDpm, iFileId, &FileDir))
  {
    return FALSE;
  }

  // サイズチェック
  if(UnDpmGetOriginalSize(hDpm, iFileId) < dwSize)
  {
    return FALSE;
  }

  // メモリに解凍
  UnDpmDecrypt(lpBuffer, (LPBYTE)UnDpmGetDataSection(hDpm) + FileDir.dwOffsetInDS, dwSize, FileDir.dwCryptKey, 0x00);

  // 解凍終了
  return TRUE;
}

/*--------------------------------------------------------------------------
  概要:         DPMINTERNALINFO 構造体へのポインタを取得します。

  宣言:         LPDPMINTERNALINFO UnDpmGetInternalInfo(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       関数が成功すると、指定されたハンドルに関連付けられた 
                DPMINTERNALINFO 構造体へのポインタが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         指定されたハンドルに関連付けられた DPMINTERNALINFO 構造体へ
                のポインタを取得します。
--------------------------------------------------------------------------*/
LPDPMINTERNALINFO UnDpmGetInternalInfo(HDPM hDpm)
{
  return (LPDPMINTERNALINFO)hDpm;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブのヘッダ情報を取得します。

  宣言:         BOOL UnDpmGetDpmHed(
                    HDPM hDpm,      // アーカイブハンドル
                    LPDPMHED lpHed  // ヘッダ情報を格納するバッファ
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                lpHed
                    ヘッダ情報を格納するバッファのポインタを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         アーカイブのヘッダ情報を取得します。
--------------------------------------------------------------------------*/
BOOL UnDpmGetDpmHed(HDPM hDpm, LPDPMHED lpHed)
{
  // ヘッダ情報をコピー
  memcpy(lpHed, UnDpmGetDpmByHandle(hDpm), sizeof(DPMHED));

  return TRUE;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブ内ファイルの情報を取得します。

  宣言:         BOOL UnDpmGetFileDir(
                    HDPM hDpm,              // アーカイブハンドル
                    DWORD iFileId,          // ファイルを表す数値
                    LPDPMFILEDIR lpFileDir  // ファイル情報を格納するバッファ
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイルを表す数値を指定します。

                    UnDpmFilenameToId 関数を使ってファイル名からこの数値を
                    取得することができます。

                lpFileDir
                    ファイル情報を格納するバッファのポインタを指定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         アーカイブ内ファイルの情報を取得します。
--------------------------------------------------------------------------*/
BOOL UnDpmGetFileDir(HDPM hDpm, DWORD iFileId, LPDPMFILEDIR lpFileDir)
{
  LPDPMFILEDIR lpFileDirSrc;  // ファイル情報ポインタ

  // ファイル情報を取得
  lpFileDirSrc = UnDpmGetFileDirPtr(hDpm, iFileId);
  if(lpFileDirSrc == NULL)
  {
    return FALSE;
  }

  // ファイル情報をコピー
  memcpy(lpFileDir, lpFileDirSrc, sizeof(DPMFILEDIR));

  // 成功
  return TRUE;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブ内ファイルの情報へのポインタを取得します。

  宣言:         LPDPMFILEDIR UnDpmGetFileDirPtr(
                    HDPM hDpm,              // アーカイブハンドル
                    DWORD iFileId           // ファイルを表す数値
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイルを表す数値を指定します。

                    UnDpmFilenameToId 関数を使ってファイル名からこの数値を
                    取得することができます。

  戻り値:       関数が成功すると、ファイル情報へのポインタが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         アーカイブ内ファイルの情報領域へのポインタを取得します。
                この関数は内容を書き換える際に内部的に使用されるものです。
                情報の取得目的には UnDpmGetFileDir を使用してください。
--------------------------------------------------------------------------*/
static LPDPMFILEDIR UnDpmGetFileDirPtr(HDPM hDpm, DWORD iFileId)
{
  LPDPMHED lpDpmHed;          // ヘッダ情報
  LPDPMFILEDIR lpFirstFile;   // 最初のファイル情報

  // ヘッダ情報を取得
  lpDpmHed = (LPDPMHED)UnDpmGetDpmByHandle(hDpm);

  // ファイル ID チェック
  if(!UnDpmIsFileId(hDpm, iFileId))
  {
    return NULL;
  }

  // 最初のファイル情報を取得
  lpFirstFile = (LPDPMFILEDIR)(lpDpmHed + 1);

  // ファイル情報へのポインタを返す
  return lpFirstFile + (iFileId - 1);
}

/*--------------------------------------------------------------------------
  概要:         データセクションへのポインタを取得します。

  宣言:         LPVOID UnDpmGetDataSection(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       関数が成功すると、データセクションへのポインタが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         データセクションへのポインタを取得します。
--------------------------------------------------------------------------*/
LPVOID UnDpmGetDataSection(HDPM hDpm)
{
  LPDPMHED lpDpmHed = (LPDPMHED)UnDpmGetDpmByHandle(hDpm);    // ヘッダ情報

  return (LPBYTE)lpDpmHed + lpDpmHed->dwOffsetToDS;
}

/*--------------------------------------------------------------------------
  概要:         ファイル ID が正常かどうか調べます。

  宣言:         BOOL UnDpmIsFileId(
                    HDPM hDpm,      // アーカイブハンドル
                    DWORD iFileId   // ファイル ID
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

  戻り値:       正常なファイル ID ならば、0 以外の値が返ります。

                異常なファイル ID ならば、0 が返ります。

  解説:         ファイル ID が正常かどうか調べます。
--------------------------------------------------------------------------*/
BOOL UnDpmIsFileId(HDPM hDpm, DWORD iFileId)
{
  if(iFileId != DPM_INVALID_FILE_ID && iFileId <= UnDpmGetFileCount(hDpm))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*--------------------------------------------------------------------------
  概要:         ハンドルからアーカイブデータのポインタを取得します。

  宣言:         LPVOID UnDpmGetDpmByHandle(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       関数が成功すると、アーカイブデータのポインタが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         ハンドルからアーカイブデータのポインタを取得します。
--------------------------------------------------------------------------*/
static LPVOID UnDpmGetDpmByHandle(HDPM hDpm)
{
  return (LPVOID)((LPDPMINTERNALINFO)hDpm + 1);
}

/*--------------------------------------------------------------------------
  概要:         DPM アーカイブをメモリから開きます。

  宣言:         HDPM UnDpmOpenDpmMem(
                    LPVOID lpBuffer,    // バッファへのポインタ
                    DWORD dwSize        // アーカイブのサイズ (バイト単位)
                );

  パラメータ:   lpBuffer
                    アーカイブを保持しているバッファへのポインタを指定しま
                    す。

                dwSize
                    アーカイブのサイズをバイト単位で指定します。

  戻り値:       関数が成功すると、アーカイブのハンドルが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         アーカイブの操作を行うためのハンドルを取得します。

                この関数では実行ファイル形式のアーカイブを開くことはできま
                せん。実行ファイル形式は UnDpmOpenExeMem 関数を使って開くこ
                とができます。
--------------------------------------------------------------------------*/
static HDPM UnDpmOpenDpmMem(LPVOID lpBuffer, DWORD dwSize)
{
  HDPM hDpm;                          // アーカイブハンドル
  DWORD dwArcSize;                    // アーカイブサイズ
  LPDPMINTERNALINFO lpInternalInfo;   // 内部情報

  // アーカイブチェック
  if(!UnDpmCheckDpmMem(lpBuffer, dwSize, &dwArcSize))
  {
    return NULL;
  }

  // メモリブロックを割り当て
  hDpm = MemoryAlloc(dwArcSize + sizeof(DPMINTERNALINFO));

  // 正常にメモリが割り当てられたか
  if(hDpm)
  {
    // ファイルデータをコピー
    memcpy(UnDpmGetDpmByHandle(hDpm), lpBuffer, dwArcSize);

    // アーカイブ情報をクリア
    lpInternalInfo = UnDpmGetInternalInfo(hDpm);
    ZeroMemory(lpInternalInfo, sizeof(DPMINTERNALINFO));
    lpInternalInfo->dwFileSize = dwArcSize;
    lpInternalInfo->dwVersion = UnDpmCheckArcVersion(hDpm);
    lpInternalInfo->bExecutable = FALSE;
  }

  return hDpm;
}

/*--------------------------------------------------------------------------
  概要:         実行ファイル形式のアーカイブをメモリから開きます。

  宣言:         HDPM UnDpmOpenExeMem(
                    LPVOID lpBuffer,    // バッファへのポインタ
                    DWORD dwSize        // アーカイブのサイズ (バイト単位)
                );

  パラメータ:   lpBuffer
                    アーカイブを保持しているバッファへのポインタを指定しま
                    す。

                dwSize
                    アーカイブのサイズをバイト単位で指定します。

  戻り値:       関数が成功すると、アーカイブのハンドルが返ります。

                関数が失敗すると、NULL が返ります。

  解説:         アーカイブの操作を行うためのハンドルを取得します。

                この関数では単体ファイル形式のアーカイブを開くことはできま
                せん。単体ファイル形式は UnDpmOpenDpmMem 関数を使って開くこ
                とができます。
--------------------------------------------------------------------------*/
static HDPM UnDpmOpenExeMem(LPVOID lpBuffer, DWORD dwSize)
{
  HDPM hDpm = NULL;                   // アーカイブハンドル
#ifdef UNDPM32_ALLOWEXEFILE
  DWORD dwDpmSize;                    // DPM のサイズ
  LPHSPHED lpHspHed;                  // HSPHED の位置を示すポインタ
  LPDPMHED lpDpmHed;                  // DPMHED の位置を示すポインタ
  LPDPMINTERNALINFO lpInternalInfo;   // DPMINTERNALINFO の位置を示すポインタ

  // アーカイブチェック
  if(UnDpmCheckExeMem(lpBuffer, dwSize, &lpHspHed, &lpDpmHed, &dwDpmSize))
  {
    // メモリから DPM 読み込み
    hDpm = UnDpmOpenDpmMem(lpDpmHed, dwDpmSize);

    // 正常に読み込めたか
    if(hDpm)
    {
      // 内部情報
      lpInternalInfo = UnDpmGetInternalInfo(hDpm);

      // 実行ファイル形式
      lpInternalInfo->bExecutable = TRUE;

      // 実行ファイルの鍵
      lpInternalInfo->dwExeFileKey = lpHspHed->dwCryptKey;
    }
  }
#endif
  return hDpm;
}

/*--------------------------------------------------------------------------
  概要:         DPM アーカイブの正当性を検査します。

  宣言:         BOOL UnDpmCheckDpmMem(
                    LPVOID lpBuffer,    // バッファへのポインタ
                    DWORD dwSize,       // アーカイブのサイズ (バイト単位)
                    LPDWORD lpdwArcSize // アーカイブのサイズを受け取る変数
                );

  パラメータ:   lpBuffer
                    アーカイブを保持しているバッファへのポインタを指定しま
                    す。

                dwSize
                    アーカイブのサイズをバイト単位で指定します。
                    アーカイブのサイズよりも大きい値でも構いません。

                lpdwArcSize
                    アーカイブの実際のサイズを取得する変数へのポインタを指
                    定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         アーカイブの正当性を検査します。
--------------------------------------------------------------------------*/
static BOOL UnDpmCheckDpmMem(LPVOID lpBuffer, DWORD dwSize, LPDWORD lpdwArcSize)
{
  DWORD i;    // 制御変数

  LPDPMHED lpDpmHed = lpBuffer;   // ヘッダへのポインタ
  LPDPMFILEDIR lpFileDir;         // ファイル情報へのポインタ
  DWORD dwRequiredSize;           // アーカイブに必要なサイズ

  // サイズチェック (ヘッダ)
  if(dwSize < sizeof(DPMHED))
  {
    return FALSE;
  }

  // シグネチャのチェック
  if(memcmp(lpDpmHed->sSignature, DPM_SIGNATURE, 4) != 0)
  {
    return FALSE;
  }

  // サイズチェック (ファイル情報)
  if(dwSize < sizeof(DPMHED) + sizeof(DPMFILEDIR) * lpDpmHed->nNumberOfFile)
  {
    return FALSE;
  }

  // ファイル情報へのポインタをセット
  lpFileDir = (LPDPMFILEDIR)(lpDpmHed + 1);

  // すべてのファイルをチェック
  *lpdwArcSize = sizeof(DPMHED) + sizeof(DPMFILEDIR) * lpDpmHed->nNumberOfFile;
  for(i = 0; i < lpDpmHed->nNumberOfFile; i++)
  {
    // 必要なサイズを計算
    dwRequiredSize = lpDpmHed->dwOffsetToDS             // データセクション位置
                     + (lpFileDir + i)->dwOffsetInDS    // ファイルデータ位置
                     + (lpFileDir + i)->dwFileSize      // ファイルサイズ
    ;

    // サイズチェック
    if(dwSize < dwRequiredSize)
    {
      return FALSE;
    }

    // アーカイブのサイズを更新
    if(dwRequiredSize > *lpdwArcSize)
    {
      *lpdwArcSize = dwRequiredSize;
    }
  }

  return TRUE;
}

/*--------------------------------------------------------------------------
  概要:         実行ファイル形式のアーカイブの正当性を検査します。

  宣言:         BOOL UnDpmCheckExeMem(
                    LPVOID lpBuffer,    // バッファへのポインタ
                    DWORD dwSize,       // アーカイブのサイズ (バイト単位)
                    LPHSPHED *lpHspHed, // HSPHED の位置を受け取るポインタ
                    LPDPMHED *lpDpmHed  // DPMHED の位置を受け取るポインタ
                );

  パラメータ:   lpBuffer
                    アーカイブを保持しているバッファへのポインタを指定しま
                    す。

                dwSize
                    アーカイブのサイズをバイト単位で指定します。
                    アーカイブのサイズよりも大きい値でも構いません。

                lpHspHed
                    HSPHED の位置を受け取るポインタへのポインタを指定します。

                lpDpmHed
                    DPMHED の位置を受け取るポインタへのポインタを指定します。

                lpdwArcSize
                    アーカイブの実際のサイズを取得する変数へのポインタを指
                    定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         アーカイブの正当性を検査します。

                HSPHED は実行ファイルの動作に関する情報が格納された領域です。
                DPM の位置や暗号鍵などの情報を得ることができます。
--------------------------------------------------------------------------*/
static BOOL UnDpmCheckExeMem(LPVOID lpBuffer, DWORD dwSize, LPHSPHED *lpHspHed, LPDPMHED *lpDpmHed, LPDWORD lpdwArcSize)
{
#ifdef UNDPM32_ALLOWEXEFILE
  LPVOID lpPEHeader;

  // サイズチェック
  if(dwSize < 0x40)
  {
    return FALSE;
  }

  // 実行ファイルか
  if(memcmp(lpBuffer, "MZ", 2) != 0)
  {
    return FALSE;
  }

  // PE (Portable Executable) Header の位置を取得
  lpPEHeader = (LPBYTE)lpBuffer + *(LPDWORD)((LPBYTE)lpBuffer + 0x3C);

  // PE か
  if(memcmp(lpPEHeader, "PE\0\0", 4) != 0)
  {
    return FALSE;
  }

  // HSPHED の位置を取得
  if(!UnDpmSearchHspHed(lpBuffer, dwSize, lpHspHed, lpDpmHed, lpdwArcSize))
  {
    return FALSE;
  }

  return TRUE;
#else
  return FALSE;
#endif
}

/*--------------------------------------------------------------------------
  概要:         HSPHED の位置を取得します。

  宣言:         BOOL UnDpmSearchHspHed(
                    LPVOID lpBuffer,    // バッファへのポインタ
                    DWORD dwSize,       // アーカイブのサイズ (バイト単位)
                    LPHSPHED *lpHspHed, // HSPHED の位置を受け取るポインタ
          LPDPMHED *lpDpmHed, // DPM のオフセットを受け取る変数
                    LPDWORD lpdwArcSize // アーカイブのサイズを受け取る変数
                );

  パラメータ:   lpBuffer
                    アーカイブを保持しているバッファへのポインタを指定しま
                    す。

                dwSize
                    アーカイブのサイズをバイト単位で指定します。
                    アーカイブのサイズよりも大きい値を指定しても構いません。

                lpHspHed
                    HSPHED の位置を受け取るポインタへのポインタを指定します。

                lpDpmHed
                    DPM の位置を受け取るポインタへのポインタを指定します。
                    この値は lpHspHed が示す HSPHED 構造体の値に基づきます。

                lpdwArcSize
                    アーカイブの実際のサイズを取得する変数へのポインタを指
                    定します。

  戻り値:       関数が成功すると、0 以外の値が返ります。

                関数が失敗すると、0 が返ります。

  解説:         HSPHED の位置を取得します。

                HSPHED は実行ファイルの動作に関する情報が格納された領域です。
                DPM の位置や暗号鍵などの情報を得ることができます。
--------------------------------------------------------------------------*/
static BOOL UnDpmSearchHspHed(LPVOID lpBuffer, DWORD dwSize, LPHSPHED *lpHspHed, LPDPMHED *lpDpmHed, LPDWORD lpdwArcSize)
{
  DWORD i;                // 制御変数
  CHAR szDpmOffsetA[64];  // DPM へのオフセット文字列 (ANSI)
  UINT nDpmOffset;        // DPM へのオフセット
  LPHSPHED lpHspHedTemp;  // HSPHED へのポインタ
  LPDPMHED lpDpmHedTemp;  // DPMHED へのポインタ

  // DPMと実行ファイル固有の情報を検索
  for(i = 0; i <= dwSize - sizeof(HSPHED); i++)
  {
    BYTE c;

    // HSPHED の候補位置を格納 (バージョン間差異の都合で総当たり)
    lpHspHedTemp = (LPHSPHED)((LPBYTE)lpBuffer + i);
    // 数値っぽい？
    c = lpHspHedTemp->sDpmOffset[0];
    if((c >= '0' && c <= '9') || c == '+' || c == '-')
    {
      // DPM のオフセットを取得
      lstrcpynA(szDpmOffsetA, lpHspHedTemp->sDpmOffset, sizeof(lpHspHedTemp->sDpmOffset) + 1);
      // 文字列から数値に変換
      nDpmOffset = StrToIntA(szDpmOffsetA) + 0x10000;
      // DPMHED の候補位置を格納
      lpDpmHedTemp = (LPDPMHED)((LPBYTE)lpBuffer + nDpmOffset);

      // オフセットの範囲チェック
      if(nDpmOffset < dwSize)
      {
        // DPM をチェック
        if(UnDpmCheckDpmMem(lpDpmHedTemp, dwSize - nDpmOffset, lpdwArcSize))
        {
          *lpHspHed = lpHspHedTemp;
          *lpDpmHed = lpDpmHedTemp;
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブのバージョンを取得します。

  宣言:         DWORD UnDpmCheckArcVersion(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       アーカイブのバージョンを示す値が返ります。

  解説:         アーカイブのバージョンを取得します。
--------------------------------------------------------------------------*/
static DWORD UnDpmCheckArcVersion(HDPM hDpm)
{
  DPMHED DpmHed;          // ヘッダ情報
  DPMFILEDIR FileDir;     // ファイル情報
  BOOL bNoCrypt = TRUE;   // 暗号化されているか
  BOOL bOldCrypt = TRUE;  // 古い暗号化方式か
  DWORD i;                // 制御変数

  // ヘッダ情報を取得
  if(UnDpmGetDpmHed(hDpm, &DpmHed))
  {
    // すべてのファイルをチェック
    for(i = 1; i <= DpmHed.nNumberOfFile; i++)
    {
      // ファイル情報を取得
      if(UnDpmGetFileDir(hDpm, i, &FileDir))
      {
        // 暗号化されているか
        if(FileDir.dwCryptKey != CRYPT_DPMFILE_KEY_NOCRYPT)
        {
          bNoCrypt = FALSE;

          // 2.55 の固定暗号鍵か
          if(FileDir.dwCryptKey != CRYPT_DPMFILE_KEY_VER255)
          {
            bOldCrypt = FALSE;
          }
        }
      }
    }
  }

  // 暗号化されているか
  if(bNoCrypt)
  {
    return DPM_VERSION_HSP2X;
  }
  else{
    // 古い暗号化方式か
    if(bOldCrypt)
    {
      return DPM_VERSION_HSP255;
    }
    else
    {
      return DPM_VERSION_HSP26;
    }
  }
}
