/**
 * Susie plug-in: HSP DPM archive extractor
 * written by gocha, feel free to redistribute
 * 
 * based on spi00am_ex.h by Shimitei
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 * 
 * 実行ファイルサポートの有無はundpm32.hの宣言で決まります
 */

#ifndef axdpm_h
#define axdpm_h

#include <windows.h>
#include "spi00am.h"
#include "undpm32/undpm32.h"

/*-------------------------------------------------------------------------*/
// このPluginの情報
/*-------------------------------------------------------------------------*/
static const char *pluginfo[] = {
  "00AM",                           /* Plug-in APIバージョン */
  "HSP DPM extractor by gocha",     /* Plug-in名、バージョン及び copyright */
#ifdef UNDPM32_ALLOWEXEFILE
  "*.dpm;*.exe;*.scr",              /* 代表的な拡張子 ("*.JPG" "*.RGB;*.Q0" など) */
#else
  "*.dpm",                          /* 代表的な拡張子 ("*.JPG" "*.RGB;*.Q0" など) */
#endif
#ifdef HSP33_SUPPORT
  "Hot Soup Processor DPM (3.3+)",  /* ファイル形式名 */
#else
  "Hot Soup Processor DPM (2.6+)",  /* ファイル形式名 */
#endif
};

// ヘッダチェック等に必要なサイズ.2KB以内で
#define HEADBUF_SIZE  sizeof(0x10)

BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
BOOL IsSupportedEx(char *filename, char *data);
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf);
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo, SPI_PROGRESS lpPrgressCallback, long lData);

#endif
