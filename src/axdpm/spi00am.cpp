/**
 * SUSIE32 '00AM' Plug-in Sample by shimitei (modified by gocha)
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 */

#include <windows.h>
#include <string.h>
#include "spi00am.h"
#include "axdpm.h"
#include "infcache.h"

//グローバル変数
InfoCache infocache; //アーカイブ情報キャッシュクラス

//---------------------------------------------------------------------------
/* エントリポイント */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call) {
    case DLL_PROCESS_DETACH:
      infocache.Clear();
      break;
  }
  return SpiEntryPoint(hModule, ul_reason_for_call, lpReserved);
}

/***************************************************************************
 * SPI関数
 ***************************************************************************/
//---------------------------------------------------------------------------
int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen)
{
  if (infono < 0 || infono >= (sizeof(pluginfo) / sizeof(char *))) 
    return 0;

  strncpy(buf, pluginfo[infono], buflen);

  return (int) strlen(buf);
}
//---------------------------------------------------------------------------
int __stdcall IsSupported(LPSTR filename, DWORD dw)
{
  char *data;
  char buff[HEADBUF_SIZE];

  if ((dw & 0xFFFF0000) == 0) {
  /* dwはファイルハンドル */
    DWORD ReadBytes;
    if (!ReadFile((HANDLE)dw, buff, HEADBUF_SIZE, &ReadBytes, NULL)) {
      return 0;
    }
    data = buff;
  } else {
  /* dwはバッファへのポインタ */
    data = (char *)dw;
  }

  /* フォーマット確認 */
  if (IsSupportedEx(filename, data)) return 1;

  return 0;
}
//---------------------------------------------------------------------------
//アーカイブ情報をキャッシュする
int GetArchiveInfoCache(char *filename, long len, HLOCAL *phinfo, fileInfo *pinfo)
{
  int ret = infocache.Dupli(filename, phinfo, pinfo);
  if (ret != SPI_NO_FUNCTION) return ret;

  //キャッシュに無い
  HLOCAL hinfo;
  HANDLE hf;
  char headbuf[HEADBUF_SIZE];
  DWORD ReadBytes;

  hf = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (hf == INVALID_HANDLE_VALUE) return SPI_FILE_READ_ERROR;
  if (SetFilePointer(hf, len, NULL, FILE_BEGIN) != (DWORD)len) {
    CloseHandle(hf);
    return SPI_FILE_READ_ERROR;
  }
  if (!ReadFile(hf, headbuf, HEADBUF_SIZE, &ReadBytes, NULL)) {
    CloseHandle(hf);
    return SPI_FILE_READ_ERROR;
  }
  CloseHandle(hf);
  if (ReadBytes != HEADBUF_SIZE) return SPI_NOT_SUPPORT;
  if (!IsSupportedEx(filename, headbuf)) return SPI_NOT_SUPPORT;
  
  ret = GetArchiveInfoEx(filename, len, &hinfo);
  if (ret != SPI_ALL_RIGHT) return ret;

  //キャッシュ
  infocache.Add(filename, &hinfo);

if (phinfo != NULL) {
  UINT size = (UINT) LocalSize(hinfo);
  /* 出力用のメモリの割り当て */
  *phinfo = LocalAlloc(LMEM_FIXED, size);
  if (*phinfo == NULL) {
    return SPI_NO_MEMORY;
  }

  memcpy(*phinfo, (void*)hinfo, size);
} else {
  fileInfo *ptmp = (fileInfo *)hinfo;
  if (pinfo->filename[0] != '\0') {
    for (;;) {
      if (ptmp->method[0] == '\0') return SPI_NO_FUNCTION;
      if (_stricmp(ptmp->filename, pinfo->filename) == 0) break;
      ptmp++;
    }
  } else {
    for (;;) {
      if (ptmp->method[0] == '\0') return SPI_NO_FUNCTION;
      if (ptmp->position == pinfo->position) break;
      ptmp++;
    }
  }
  *pinfo = *ptmp;
}
  return SPI_ALL_RIGHT;
}
//---------------------------------------------------------------------------
int __stdcall GetArchiveInfo(LPSTR buf, long len, unsigned int flag, HLOCAL *lphInf)
{
  //メモリ入力には対応しない
  if ((flag & 7) != 0) return SPI_NO_FUNCTION;

  *lphInf = NULL;
  return GetArchiveInfoCache(buf, len, lphInf, NULL);
}
//---------------------------------------------------------------------------
int __stdcall GetFileInfo
(LPSTR buf, long len, LPSTR filename, unsigned int flag, struct fileInfo *lpInfo)
{
//  bool caseSensitive = !(flag & 0x80);

  //メモリ入力には対応しない
  if ((flag & 7) != 0) return SPI_NO_FUNCTION;

  strcpy(lpInfo->filename, filename);

  return GetArchiveInfoCache(buf, len, NULL, lpInfo);
}
//---------------------------------------------------------------------------
int __stdcall GetFile(LPSTR src, long len,
         LPSTR dest, unsigned int flag,
         SPI_PROGRESS lpPrgressCallback, long lData)
{
  //ファイルへの出力は対応していない
  if ((flag & 0x700) == 0) return SPI_NO_FUNCTION;
  //メモリ入力には対応しない
  if ((flag & 7) != 0) return SPI_NO_FUNCTION;

  fileInfo info;
  info.filename[0] = '\0';
  info.position = len;
  int ret = GetArchiveInfoCache(src, 0, NULL, &info);
  if (ret != SPI_ALL_RIGHT) return ret;

  return GetFileEx(src, (HLOCAL *)dest, &info, lpPrgressCallback, lData);
}
//---------------------------------------------------------------------------
