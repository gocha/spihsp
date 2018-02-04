/*--------------------------------------------------------------------------
  undpm32.h     UNDPM32.DLL header file
--------------------------------------------------------------------------*/

#ifndef __UNDPM32_H__
#define __UNDPM32_H__

#include <windows.h>

// ���s�t�@�C���W�J�p�̃R�[�h���g�p���邩�ۂ�
//#define UNDPM32_ALLOWEXEFILE

// HSP3.3�����̓W�J�R�[�h���g�p���邩�ۂ�(����HSP3.3�ȑO�Ɣr�����p)
//#define HSP33_SUPPORT

/*--------------------------------------------------------------------------
  �萔��`
--------------------------------------------------------------------------*/
#define DPM_INVALID_FILE_ID     0       // �s���ȃt�@�C�� ID

#define DPM_VERSION_HSP2X       0x0200  // HSP 2.x basic
#define DPM_VERSION_HSP255      0x0255  // HSP 2.55 �`
#define DPM_VERSION_HSP26       0x0260  // HSP 2.6 �`

/*--------------------------------------------------------------------------
  �}�N����`
--------------------------------------------------------------------------*/

#ifdef __cplusplus
#define EXTERN_C            extern "C"
#else
#define EXTERN_C            extern
#endif // defined(__cplusplus)

#ifdef UNDPM32_NO_EXTERN
#define UNDPMAPI            EXTERN_C
#else
#ifdef UNDPM32_EXPORTS
#define UNDPMAPI            EXTERN_C __declspec(dllexport)
#else
#define UNDPMAPI            EXTERN_C __declspec(dllimport)
#endif // !defined(UNDPM32_EXPORTS)
#endif // !defined(UNDPM32_NO_EXTERN)

/*--------------------------------------------------------------------------
  �^��`
--------------------------------------------------------------------------*/
typedef LPVOID  HDPM;   // �A�[�J�C�u�n���h��

/*--------------------------------------------------------------------------
  �֐��錾
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
  �t�@�C���I��
--------------------------------------------------------------------------*/
#endif // !defined(__UNDPM32_H__)