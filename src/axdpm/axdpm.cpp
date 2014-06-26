/**
 * Susie plug-in: HSP DPM archive extractor
 * written by gocha, feel free to redistribute
 * 
 * based on spi00am_ex.cpp by Shimitei
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 * 
 * SusieプラグインとUNDPM32.DLLの作りがめがっさ逆なのに加え、
 * 各種処理の手抜きが見事な低速処理を実現しています！ :/
 */

#include <windows.h>
#include <stdlib.h>
#include <memory.h>
#include "spi00am.h"
#include "axdpm.h"
#include "undpm32/undpm32.h"

/**
 * エントリポイント
 */
BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;

  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

/**
 * ファイル先頭2KB以内から対応フォーマットか判断
 * (判断に使用するサイズはヘッダファイルで定義)
 * ファイル名も判断材料として渡されているみたい
 */
BOOL IsSupportedEx(char *filename, char *data)
{
  const BYTE dpmHdrSig[4] = { 'D', 'P', 'M', 'X' };
  const BYTE exeHdrSig[4] = { 'M', 'Z', 0x90, 0x00 };

  // 手抜き、一次予選は簡単に通過できます :P
  if(memcmp(data, dpmHdrSig, 4) == 0)
  {
    return TRUE;
  }
#ifdef UNDPM32_ALLOWEXEFILE
  else if(memcmp(data, exeHdrSig, 4) == 0)
  {
    return TRUE;
  }
#endif
  else
  {
    return FALSE;
  }
}

/**
 * アーカイブ内全ファイルの情報取得
 * filenameの位置lenを先頭とみなしてアーカイブにファイル情報を格納
 * 高速化という名の悪あがきのため、実質的にlenは0以外で使えません
 */
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf)
{
  int result = SPI_ALL_RIGHT;
  HDPM dpm;

  // おねえさまがいないとわたし…なにもできないから
  dpm = UnDpmOpenArchive(filename, (DWORD) len);
  if(dpm)
  {
    DWORD nFiles;
    fileInfo* pInfo;

    // ファイルの数を教えてせんせいさん
    nFiles = UnDpmGetFileCount(dpm);

    // 返却するファイル情報の割り当て (ファイル数+1個, 初期化必須)
    pInfo = (fileInfo*) LocalAlloc(LPTR, sizeof(fileInfo) * (nFiles+1));
    if(pInfo)
    {
      DWORD fileId;

      *lphInf = (HLOCAL) pInfo;
      // 各ファイルを連続的に処理
      for(fileId = 1; fileId <= nFiles; fileId++)
      {
        // 圧縮法: 7文字以内でテケトーに
        strcpy((char*) pInfo->method, "HSP DPM");
        // 位置: ファイルを識別し、高速に処理するための鍵にすると良い
        // ※本プラグインでは仕方なく、UNDPM32用のファイルIDを入れる
        pInfo->position = fileId;
        // 圧縮されたサイズ: 無圧縮
        // 返す値はヘッダなどのサイズも含めるべきらしいです
        // 返すべき値はおねえさまに訊いてみないとわかりません
        pInfo->compsize = UnDpmGetCompressedSize(dpm, fileId);
        // 元のファイルのサイズ: 無圧縮
        pInfo->filesize = UnDpmGetOriginalSize(dpm, fileId);
        // ファイルの更新日時: 記録なし
        pInfo->timestamp = 0;
        // 相対パス: ツリー記録不可
        pInfo->path[0] = '\0';
        // ファイル名
        UnDpmGetFileNameA(dpm, fileId, pInfo->filename, 200);
        // CRC: 記録なし
        pInfo->crc = 0;

        pInfo++;
      }
    }
    else
    {
      result = SPI_NO_MEMORY;
    }
    // また後で会いましょう
    UnDpmCloseArchive(dpm);
  }
  else
  {
    // おねえさまに見捨てられてBADEND
    result = SPI_FILE_READ_ERROR;
  }
  return result;
}

/**
 * filenameにあるfileInfoのファイルを読み込む
 * デコードされたファイルを格納した空間をdestに渡す
 */
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo,
    SPI_PROGRESS lpPrgressCallback, long lData)
{
  int result = SPI_ALL_RIGHT;
  HDPM dpm;

  // おねえさまがいないとわたし…なにもできないから
  dpm = UnDpmOpenArchive(filename, 0);
  if(dpm)
  {
    DWORD fileId = (DWORD) pinfo->position;
    DWORD dataSize = (DWORD) pinfo->filesize;
    LPBYTE data;

    // ファイルバッファを確保
    data = (LPBYTE) LocalAlloc(LMEM_FIXED, dataSize);
    if(data)
    {
      // ファイル読み込み
      if(UnDpmExtractMem(dpm, fileId, data, dataSize))
      {
        *dest = (HLOCAL) data;
      }
      else
      {
        result = SPI_FILE_READ_ERROR;
      }
    }
    else
    {
      result = SPI_NO_MEMORY;
    }
    // おやすみなさい
    UnDpmCloseArchive(dpm);
  }
  else
  {
    // おねえさまに見捨てられてBADEND
    result = SPI_FILE_READ_ERROR;
  }
  return result;
}
