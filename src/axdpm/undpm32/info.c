/*--------------------------------------------------------------------------
  info.c        UNDPM32.DLL info functions
--------------------------------------------------------------------------*/

#include <windows.h>
#include <shlwapi.h>
#include <stdlib.h>

#include "undpm32.h"
#include "main.h"
#include "info.h"
#include "crypt.h"

#include "egcnvstr.h"
#include "egstring.h"
#include "egalloc.h"

/*--------------------------------------------------------------------------
  概要:         アーカイブのバージョンを取得します。

  宣言:         DWORD UnDpmGetArcVersion(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       アーカイブのバージョンを示す値が返ります。

  解説:         アーカイブのバージョンを取得します。
--------------------------------------------------------------------------*/
DWORD UnDpmGetArcVersion(HDPM hDpm)
{
  return UnDpmGetInternalInfo(hDpm)->dwVersion;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブに格納されているファイル数を取得します。

  宣言:         DWORD UnDpmGetFileCount(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       アーカイブに格納されているファイル数が返ります。

  解説:         アーカイブに格納されているファイル数を取得します。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetFileCount(HDPM hDpm)
{
  DPMHED DpmHed;  // ヘッダ情報

  // ヘッダ情報を取得
  UnDpmGetDpmHed(hDpm, &DpmHed);

  // ファイル数を返す
  return DpmHed.nNumberOfFile;
}

/*--------------------------------------------------------------------------
  概要:         アーカイブのサイズを取得します。

  宣言:         DWORD UnDpmGetArcFileSize(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       アーカイブのサイズが返ります。

  解説:         アーカイブのサイズを取得します。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetArcFileSize(HDPM hDpm)
{
  return UnDpmGetInternalInfo(hDpm)->dwFileSize;
}

/*--------------------------------------------------------------------------
  概要:         検索にヒットしたファイルの解凍後のサイズの合計を得ます。

  宣言:         DWORD UnDpmGetArcOriginalSize(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       解凍後のサイズの合計が返ります。

  解説:         UnDpmFindFirstFile および UnDpmFindNextFile による検索に
                ヒットしたファイルの解凍後のサイズの合計を得ます。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetArcOriginalSize(HDPM hDpm)
{
  return UnDpmGetInternalInfo(hDpm)->dwOriginalSizeTotal;
}

/*--------------------------------------------------------------------------
  概要:         検索にヒットしたファイルの圧縮サイズの合計を得ます。

  宣言:         DWORD UnDpmGetArcCompressedSize(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       圧縮サイズの合計が返ります。

  解説:         UnDpmFindFirstFile および UnDpmFindNextFile による検索に
                ヒットしたファイルの圧縮サイズの合計を得ます。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetArcCompressedSize(HDPM hDpm)
{
  return UnDpmGetInternalInfo(hDpm)->dwCompressedSizeTotal;
}

/*--------------------------------------------------------------------------
  概要:         実行ファイル形式かどうか調べます。

  宣言:         BOOL UnDpmIsExecutable(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

  戻り値:       実行ファイル形式ならば、0 以外の値が返ります。

                実行ファイル形式でないならば、0 が返ります。

  解説:         アーカイブが実行ファイル形式かどうか調べます。
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmIsExecutable(HDPM hDpm)
{
  return UnDpmGetInternalInfo(hDpm)->bExecutable;
}

/*--------------------------------------------------------------------------
  概要:         ファイル名から ID を取得します。 (ANSI)

  宣言:         DWORD UnDpmGetFileIdA(
                    HDPM hDpm,          // アーカイブハンドル
                    LPCSTR lpszFileName // ファイル名
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

                lpszFileName
                    ファイル名を指定します。

  戻り値:       関数が成功すると、ファイル ID が返ります。

                関数が失敗すると、NULL が返ります。

  解説:         ファイル操作を行うための ID を取得します。
                この ID を使ってファイルに操作を行います。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetFileIdA(HDPM hDpm, LPCSTR lpszFileName)
{
  DWORD iFileId;          // ファイル ID
  LPWSTR lpszFileNameW;   // Unicode に変換したファイル名

  // 文字列を Unicode に変換
  lpszFileNameW = ConvertStringAToW(lpszFileName);

  // Unicode 版の関数を呼び出し
  iFileId = UnDpmGetFileIdW(hDpm, lpszFileNameW);

  // Unicode に変換した文字列を解放
  FreeConvertedString(lpszFileNameW);

  // ファイル ID を返す
  return iFileId;
}

/*--------------------------------------------------------------------------
  概要:         ファイル名から ID を取得します。 (Unicode)

  宣言:         DWORD UnDpmGetFileIdW(
                    HDPM hDpm,              // アーカイブハンドル
                    LPCWSTR lpszFileName    // ファイル名
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

                lpszFileName
                    ファイル名を指定します。

  戻り値:       関数が成功すると、ファイル ID が返ります。

                関数が失敗すると、NULL が返ります。

  解説:         ファイル操作を行うための ID を取得します。
                この ID を使ってファイルに操作を行います。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetFileIdW(HDPM hDpm, LPCWSTR lpszFileName)
{
  DWORD iFileId = DPM_INVALID_FILE_ID;    // ファイル ID

  DWORD i;                        // 制御変数
  DPMHED DpmHed;                  // ヘッダ情報
  DPMFILEDIR FileDir;             // ファイル情報
  WCHAR szCurFileName[_MAX_PATH]; // ファイル名 (Unicode)

  // ヘッダ情報を取得
  UnDpmGetDpmHed(hDpm, &DpmHed);

  // すべてのファイルをチェック
  for(i = 1; i <= DpmHed.nNumberOfFile; i++)
  {
    // ファイル情報を取得
    if(UnDpmGetFileDir(hDpm, i, &FileDir))
    {
      // ファイル名を取得
      if(UnDpmGetFileNameW(hDpm, i, szCurFileName, countof(szCurFileName)) != 0)
      {
        // 指定されたファイルか
        if(StrCmpW(lpszFileName, szCurFileName) == 0)
        {
          iFileId = i;
          break;
        }
      }
    }
  }

  // ファイル ID を返す
  return iFileId;
}

/*--------------------------------------------------------------------------
  概要:         ID からファイル名を取得します。 (ANSI)

  宣言:         DWORD UnDpmGetFileNameA(
                    HDPM hDpm,          // アーカイブハンドル
                    DWORD iFileId,      // ファイル ID
                    LPSTR lpszBuffer,   // ファイル名を受け取るバッファ
                    DWORD cchBuffer     // バッファサイズ
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

                lpszBuffer
                    ファイル名を受け取るバッファへのポインタを指定します。

                cchBuffer
                    バッファサイズを文字数で指定します。

  戻り値:       関数が成功すると、lpszBuffer パラメータにコピーされた文字列
                の長さが文字数で返ります。この長さには、終端の NULL 文字は
                含まれません。

                lpszBuffer パラメータに指定されたバッファが小さくて、長いパ
                スを受け取ることができない場合は、長いパスを格納するために
                必要なバッファサイズが文字数で返ります。

                関数が失敗すると、0 が返ります。

  解説:         ファイル ID からファイル名を取得します。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetFileNameA(HDPM hDpm, DWORD iFileId, LPSTR lpszBuffer, DWORD cchBuffer)
{
  LPWSTR lpszBufferW;     // Unicode ファイル名取得バッファ
  DWORD cchFileName = 0;  // ファイル名の長さ

  // ファイル名領域のメモリ割り当て
  lpszBufferW = MemoryAlloc((cchBuffer + 1) * sizeof(WCHAR));

  // 正常にメモリが割り当てられたか
  if(lpszBufferW)
  {
    // ファイル名を取得
    cchFileName = UnDpmGetFileNameW(hDpm, iFileId, lpszBufferW, cchBuffer);

    // ANSI 文字列に変換
    WideCharToMultiByte(CP_ACP, 0, lpszBufferW, cchFileName + 1, lpszBuffer, cchFileName + 1, NULL, NULL);

    // メモリを解放
    MemoryFree(lpszBufferW);
  }

  // コピーされたサイズを返す
  return cchFileName;
}

/*--------------------------------------------------------------------------
  概要:         ID からファイル名を取得します。 (Unicode)

  宣言:         DWORD UnDpmGetFileNameW(
                    HDPM hDpm,          // アーカイブハンドル
                    DWORD iFileId,      // ファイル ID
                    LPWSTR lpszBuffer,  // ファイル名を受け取るバッファ
                    DWORD cchBuffer     // バッファサイズ
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

                lpszBuffer
                    ファイル名を受け取るバッファへのポインタを指定します。

                cchBuffer
                    バッファサイズを文字数で指定します。

  戻り値:       関数が成功すると、lpszBuffer パラメータにコピーされた文字列
                の長さが文字数で返ります。この長さには、終端の NULL 文字は
                含まれません。

                lpszBuffer パラメータに指定されたバッファが小さくて、長いパ
                スを受け取ることができない場合は、長いパスを格納するために
                必要なバッファサイズが文字数で返ります。

                関数が失敗すると、0 が返ります。

  解説:         ファイル ID からファイル名を取得します。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetFileNameW(HDPM hDpm, DWORD iFileId, LPWSTR lpszBuffer, DWORD cchBuffer)
{
  DPMFILEDIR FileDir;             // ファイル情報
  DWORD cchFileName = 0;          // ファイル名の長さ
  DWORD cchCopySize;              // コピーする長さ
  CHAR szFileNameA[_MAX_PATH];    // ファイル名 (ANSI)

  // 異常なバッファサイズでなければファイル情報を取得
  if(cchBuffer != 0 && UnDpmGetFileDir(hDpm, iFileId, &FileDir))
  {
    // ファイル名をコピー
    lstrcpynA(szFileNameA, FileDir.sFileName, sizeof(FileDir.sFileName) + 1);

    // ファイル名の長さを調べる
    cchFileName = lstrlenA(szFileNameA);

    // ファイル名がバッファに収まりきるか
    if(cchBuffer >= (cchFileName + 1))
    {
      // 完全に収まる場合
      cchCopySize = cchFileName;
    }
    else
    {
      // 完全には収まりきらない場合
      cchCopySize = cchBuffer - 1;
      szFileNameA[cchCopySize] = '\0';
    }

    // Unicode に変換
    MultiByteToWideChar(CP_ACP, 0, szFileNameA, cchCopySize + 1, lpszBuffer, cchCopySize + 1);
  }
  return cchFileName;
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルの解凍後のサイズを得ます。

  宣言:         DWORD UnDpmGetOriginalSize(
                    HDPM hDpm,      // アーカイブハンドル
                    DWORD iFileId   // ファイル ID
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

  戻り値:       格納ファイルの解凍後のサイズが返ります。

  解説:         格納ファイルの解凍後のサイズを得ます。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetOriginalSize(HDPM hDpm, DWORD iFileId)
{
  DPMFILEDIR FileDir; // ファイル情報

  // ファイル情報を取得
  if(UnDpmGetFileDir(hDpm, iFileId, &FileDir))
  {
    return FileDir.dwFileSize;
  }
  return 0;
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルの圧縮サイズを得ます。

  宣言:         DWORD UnDpmGetCompressedSize(
                    HDPM hDpm,      // アーカイブハンドル
                    DWORD iFileId   // ファイル ID
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

  戻り値:       格納ファイルの圧縮サイズが返ります。

  解説:         格納ファイルの圧縮サイズを得ます。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmGetCompressedSize(HDPM hDpm, DWORD iFileId)
{
  DPMFILEDIR FileDir; // ファイル情報

  // ファイル情報を取得
  if(UnDpmGetFileDir(hDpm, iFileId, &FileDir))
  {
    return FileDir.dwFileSize;
  }
  return 0;
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルが暗号化されているか調べます。

  宣言:         BOOL UnDpmIsCryptedFile(
                    HDPM hDpm,      // アーカイブハンドル
          DWORD iFileId   // ファイル ID
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

  戻り値:       暗号化されていれば、0 以外の値が返ります。

                暗号化されていなければ、0 が返ります。

  解説:         格納ファイルが暗号化されているか調べます。
                暗号化されていても UnDpmExtract などで読み出しが可能です。
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmIsCryptedFile(HDPM hDpm, DWORD iFileId)
{
  return (BOOL)(UnDpmGetCryptKey(hDpm, iFileId) != CRYPT_DPMFILE_KEY_NOCRYPT);
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルの検索を開始します。 (ANSI)

  宣言:         DWORD UnDpmFindFirstFileA(
                    HDPM hDpm,          // アーカイブハンドル
                    LPCSTR lpszWildName // 検索ファイル名
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

                lpszWildName
                    検索するファイル名を指定します。
                    ワイルドカードを使った指定が可能です。

  戻り値:       関数が成功すると、ファイル ID が返ります。

                関数が失敗すると、DPM_INVALID_FILE_ID が返ります。

  解説:         格納ファイルの検索を開始します。
                続く検索は UnDpmFindNextFile 関数を使って行います。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmFindFirstFileA(HDPM hDpm, LPCSTR lpszWildName)
{
  DWORD iFileId;          // ファイル ID
  LPWSTR lpszWildNameW;   // Unicode に変換したファイル名

  // 文字列を Unicode に変換
  lpszWildNameW = ConvertStringAToW(lpszWildName);

  // Unicode 版の関数を呼び出し
  iFileId = UnDpmFindFirstFileW(hDpm, lpszWildNameW);

  // Unicode に変換した文字列を解放
  FreeConvertedString(lpszWildNameW);

  // アーカイブハンドルを返す
  return iFileId;
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルの検索を開始します。 (Unicode)

  宣言:         DWORD UnDpmFindFirstFileW(
                    HDPM hDpm,              // アーカイブハンドル
                    LPCWSTR lpszWildName    // 検索ファイル名
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

                lpszWildName
                    検索するファイル名を指定します。
                    ワイルドカードを使った指定が可能です。

  戻り値:       関数が成功すると、ファイル ID が返ります。

                関数が失敗すると、DPM_INVALID_FILE_ID が返ります。

  解説:         格納ファイルの検索を開始します。
                続く検索は UnDpmFindNextFile 関数を使って行います。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmFindFirstFileW(HDPM hDpm, LPCWSTR lpszWildName)
{
  LPDPMINTERNALINFO lpInternalInfo = UnDpmGetInternalInfo(hDpm);  // 内部情報

  // 検索名をコピー
  lstrcpyW(lpInternalInfo->szFindName, lpszWildName);

  // 最初のファイルを設定
  lpInternalInfo->iFindFileId = 1;

  // サイズ情報を初期化
  lpInternalInfo->dwOriginalSizeTotal = 0;
  lpInternalInfo->dwCompressedSizeTotal = 0;

  return UnDpmFindNextFile(hDpm);
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルの検索をします。

  宣言:         DWORD UnDpmFindNextFile(
                    HDPM hDpm   // アーカイブハンドル
                );

  パラメータ:   hDpm
                    アーカイブハンドルを指定します。

  戻り値:       指定したファイルが見つかると、ファイル ID が返ります。

                指定したファイルが見つからなかった場合
                DPM_INVALID_FILE_ID が返ります。

  解説:         格納ファイルの検索をします。
                検索開始は UnDpmFindFirstFile 関数を使って行います。
--------------------------------------------------------------------------*/
UNDPMAPI DWORD UnDpmFindNextFile(HDPM hDpm)
{
  LPDPMINTERNALINFO lpInternalInfo;   // 内部情報
  DPMHED DpmHed;                      // ヘッダ情報
  WCHAR szFileName[_MAX_PATH];        // ファイル名

  // 内部情報を取得
  lpInternalInfo = UnDpmGetInternalInfo(hDpm);

  // ヘッダ情報を取得
  UnDpmGetDpmHed(hDpm, &DpmHed);

  // ファイルを検索
  for(; lpInternalInfo->iFindFileId <= DpmHed.nNumberOfFile; lpInternalInfo->iFindFileId++)
  {
    // ファイル名を取得
    UnDpmGetFileNameW(hDpm, lpInternalInfo->iFindFileId, szFileName, countof(szFileName));

    // 条件にあったファイル名か
    if(PathMatchSpecW(szFileName, lpInternalInfo->szFindName))
    {
      // サイズを取得する
      lpInternalInfo->dwOriginalSizeTotal += UnDpmGetOriginalSize(hDpm, lpInternalInfo->iFindFileId);
      lpInternalInfo->dwCompressedSizeTotal += UnDpmGetCompressedSize(hDpm, lpInternalInfo->iFindFileId);

      // ファイル ID を返す
      return lpInternalInfo->iFindFileId++;
    }
  }

  return DPM_INVALID_FILE_ID;
}

/*--------------------------------------------------------------------------
  概要:         格納ファイルの暗号鍵を取得します。

  宣言:         DWORD UnDpmGetCryptKey(
                    HDPM hDpm,      // アーカイブハンドル
                    DWORD iFileId   // ファイル ID
                );

  パラメータ:   hDpm
                    アーカイブのハンドルを指定します。

                iFileId
                    ファイル ID を指定します。

  戻り値:       暗号鍵が返ります。
                暗号化されていない場合、CRYPT_DPMFILE_KEY_NOCRYPT が返ります。

  解説:         格納ファイルの暗号鍵を取得します。
--------------------------------------------------------------------------*/
DWORD UnDpmGetCryptKey(HDPM hDpm, DWORD iFileId)
{
  DPMFILEDIR FileDir; // ファイル情報

  // ファイル情報を取得
  if(UnDpmGetFileDir(hDpm, iFileId, &FileDir))
  {
    return FileDir.dwCryptKey;
  }
  else
  {
    // ファイル情報取得エラー
    return CRYPT_DPMFILE_KEY_NOCRYPT;
  }
}
