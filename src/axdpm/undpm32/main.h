/*--------------------------------------------------------------------------
  main.h        UNDPM32.DLL main
--------------------------------------------------------------------------*/

#ifndef __UNDPM32_MAIN_H__
#define __UNDPM32_MAIN_H__

#include <windows.h>
#include "undpm32.h"

/*--------------------------------------------------------------------------
  型宣言
--------------------------------------------------------------------------*/

// アライメントを 1 バイトに
#pragma pack(push, 1)

typedef struct tagDPMHED {
  BYTE sSignature[4];     // シグネチャ
  DWORD dwOffsetToDS;     // データセクションへのオフセット
  DWORD nNumberOfFile;    // ファイル数
  DWORD dwReserved;       // 予約済み
} DPMHED, *LPDPMHED;

typedef struct tagDPMFILEDIR {
  BYTE sFileName[16];     // ファイル名 (NULL 文字を除く)
  DWORD dwReserved;       // 予約済み? (常に 0xFFFFFFFF)
  DWORD dwCryptKey;       // 暗号鍵
  DWORD dwOffsetInDS;     // データへのオフセット
  DWORD dwFileSize;       // データサイズ
} DPMFILEDIR, *LPDPMFILEDIR;

typedef struct tagHSPHED {
  BYTE szSignature[9];    // シグネチャ
  BYTE sDpmOffset[8];     // DPM へのオフセット
  BYTE szExeType[2];      // 実行ファイルのタイプ
  BYTE cMarkerX;          // 次のデータを示す文字 (x)
  WORD nWidth;            // ウィンドウ x サイズ
  BYTE cMarkerY;          // 次のデータを示す文字 (y)
  WORD nHeight;           // ウィンドウ y サイズ
  BYTE cMarkerD;          // 次のデータを示す文字 (d)
  WORD fDisplaySwitch;    // ウィンドウを非表示にするフラグ
  BYTE cMarkerS;          // 次のデータを示す文字 (s)
  WORD wCheckSum;         // チェックサム
  BYTE cMarkerK;          // 次のデータを示す文字 (k)
  DWORD dwCryptKey;       // 暗号鍵 (実行ファイル)
  BYTE byReserved;        // 予約済み (_)
} HSPHED, *LPHSPHED;

// アライメントを戻す
#pragma pack(pop)

typedef struct tagDPMINTERNALINFO {
  DWORD dwFileSize;               // ファイルサイズ
  DWORD dwVersion;                // アーカイブのバージョン
  BOOL bExecutable;               // 実行ファイル形式か
  DWORD dwExeFileKey;             // 実行ファイルの鍵
  DWORD iFindFileId;              // FindFirstFile で使うファイル ID
  WCHAR szFindName[_MAX_PATH];    // FindFirstFile で使う検索名
  DWORD dwOriginalSizeTotal;      // 解凍後のサイズの合計
  DWORD dwCompressedSizeTotal;    // 圧縮サイズの合計
} DPMINTERNALINFO, *LPDPMINTERNALINFO;

/*--------------------------------------------------------------------------
  関数宣言
--------------------------------------------------------------------------*/
LPDPMINTERNALINFO UnDpmGetInternalInfo(HDPM hDpm);
BOOL UnDpmGetDpmHed(HDPM hDpm, LPDPMHED lpHed);
BOOL UnDpmGetFileDir(HDPM hDpm, DWORD iFileId, LPDPMFILEDIR lpFileDir);
LPVOID UnDpmGetDataSection(HDPM hDpm);
BOOL UnDpmIsFileId(HDPM hDpm, DWORD iFileId);

/*--------------------------------------------------------------------------
  ファイル終了
--------------------------------------------------------------------------*/
#endif // !defined(__UNDPM32_MAIN_H__)