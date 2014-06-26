/*--------------------------------------------------------------------------
  undpm32.h     UNDPM32.DLL header file
--------------------------------------------------------------------------*/

#ifndef __UNDPM32_H__
#define __UNDPM32_H__

#include <windows.h>

// 実行ファイル展開用のコードを使用するか否か
#define UNDPM32_ALLOWEXEFILE

// HSP3.3向けの展開コードを使用するか否か(現状HSP3.3以前と排他利用)
//#define HSP33_SUPPORT

/*--------------------------------------------------------------------------
  定数定義
--------------------------------------------------------------------------*/
#define DPM_INVALID_FILE_ID     0       // 不正なファイル ID

#define DPM_VERSION_HSP2X       0x0200  // HSP 2.x basic
#define DPM_VERSION_HSP255      0x0255  // HSP 2.55 ～
#define DPM_VERSION_HSP26       0x0260  // HSP 2.6 ～

/*--------------------------------------------------------------------------
  マクロ定義
--------------------------------------------------------------------------*/

#ifdef __cplusplus
#define EXTERN_C            extern "C"
#else
#define EXTERN_C            extern
#endif // defined(__cplusplus)

#ifdef UNDPM32_EXPORTS
#define UNDPMAPI
#else
#define UNDPMAPI            EXTERN_C
#endif

// DLL時代の名残
#if 0
#ifdef UNDPM32_EXPORTS
#define UNDPMAPI            EXTERN_C __declspec(dllexport)
#else
#define UNDPMAPI            EXTERN_C __declspec(dllimport)
#endif // !defined(UNDPM32_EXPORTS)
#endif

/*--------------------------------------------------------------------------
  型定義
--------------------------------------------------------------------------*/
typedef LPVOID  HDPM;   // アーカイブハンドル

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
UNDPMAPI BOOL UnDpmCloseArchive(HDPM hDpm);
UNDPMAPI BOOL UnDpmExtractA(HDPM hDpm, DWORD iFileId, LPSTR lpszDirName);
UNDPMAPI BOOL UnDpmExtractW(HDPM hDpm, DWORD iFileId, LPWSTR lpszDirName);
UNDPMAPI BOOL UnDpmExtractMem(HDPM hDpm, DWORD iFileId, LPVOID lpBuffer, DWORD dwSize);
UNDPMAPI DWORD UnDpmFindFirstFileA(HDPM hDpm, LPCSTR lpszWildName);
UNDPMAPI DWORD UnDpmFindFirstFileW(HDPM hDpm, LPCWSTR lpszWildName);
UNDPMAPI DWORD UnDpmFindNextFile(HDPM hDpm);
UNDPMAPI DWORD UnDpmGetArcCompressedSize(HDPM hDpm);
UNDPMAPI DWORD UnDpmGetArcFileSize(HDPM hDpm);
UNDPMAPI DWORD UnDpmGetArcOriginalSize(HDPM hDpm);
UNDPMAPI DWORD UnDpmGetCompressedSize(HDPM hDpm, DWORD iFileId);
UNDPMAPI DWORD UnDpmGetFileCount(HDPM hDpm);
UNDPMAPI DWORD UnDpmGetFileIdA(HDPM hDpm, LPCSTR lpszFileName);
UNDPMAPI DWORD UnDpmGetFileIdW(HDPM hDpm, LPCWSTR lpszFileName);
UNDPMAPI DWORD UnDpmGetFileNameA(HDPM hDpm, DWORD iFileId, LPSTR lpszBuffer, DWORD cchBuffer);
UNDPMAPI DWORD UnDpmGetFileNameW(HDPM hDpm, DWORD iFileId, LPWSTR lpszBuffer, DWORD cchBuffer);
UNDPMAPI DWORD UnDpmGetOriginalSize(HDPM hDpm, DWORD iFileId);
UNDPMAPI BOOL  UnDpmIsCryptedFile(HDPM hDpm, DWORD iFileId);
UNDPMAPI BOOL  UnDpmIsExecutable(HDPM hDpm);
UNDPMAPI HDPM  UnDpmOpenArchiveA(LPCSTR lpszFileName, DWORD dwOffset);
UNDPMAPI HDPM  UnDpmOpenArchiveW(LPCWSTR lpszFileName, DWORD dwOffset);
UNDPMAPI HDPM  UnDpmOpenArchiveMem(LPVOID lpBuffer, DWORD dwSize);

#ifdef UNICODE
#define UnDpmExtract        UnDpmExtractW
#define UnDpmFindFirstFile  UnDpmFindFirstFileW
#define UnDpmGetFileId      UnDpmGetFileIdW
#define UnDpmGetFileName    UnDpmGetFileNameW
#define UnDpmOpenArchive    UnDpmOpenArchiveW
#else
#define UnDpmExtract        UnDpmExtractA
#define UnDpmFindFirstFile  UnDpmFindFirstFileA
#define UnDpmGetFileId      UnDpmGetFileIdA
#define UnDpmGetFileName    UnDpmGetFileNameA
#define UnDpmOpenArchive    UnDpmOpenArchiveA
#endif // defined(UNICODE)

/*--------------------------------------------------------------------------
  ファイル終了
--------------------------------------------------------------------------*/
#endif // !defined(__UNDPM32_H__)