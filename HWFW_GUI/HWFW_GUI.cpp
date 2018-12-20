﻿#include "stdafx.h"
#include "HWFW_GUI.h"


// 全局变量: 
HINSTANCE hInst = NULL;                                // 当前实例
HWND hMainDlg = NULL;
HMENU hmPop = NULL;
HMENU hmItemInfo = NULL;


//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // 将实例句柄存储在全局变量中

  DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, &DlgProc_Main);

  return TRUE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // 执行应用程序初始化: 
  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  return 1;
}

void SetStatus(LPCWSTR lpText, ...)
{
  va_list argptr;
  WCHAR wsTmp[512];

  va_start(argptr, lpText);
  _vstprintf_s(wsTmp, 512, lpText, argptr);
  va_end(argptr);

  SetWindowTextW(GetDlgItem(hMainDlg, IDC_LBL_STATUS), wsTmp);
}

void SetTooltip(HWND hCtrl, LPCWSTR lpText, ...)
{
  va_list argptr;
  WCHAR wsTmp[512];

  va_start(argptr, lpText);
  _vstprintf_s(wsTmp, 512, lpText, argptr);
  va_end(argptr);

  SetWindowTextW(hCtrl, wsTmp);
}

BOOL QueryMsg(HWND hOwner, LPCWSTR szText, LPCWSTR szTitle)
{
  if (MessageBoxW(hOwner, szText, szTitle, MB_ICONQUESTION | MB_YESNO) == IDYES)
    return TRUE;
  else
    return FALSE;
}

BOOL GetOpenFilePath(HWND hOwner, LPWSTR lpFilePath, DWORD dwMax)
{
  OPENFILENAMEW ofn;

  if (!lpFilePath || !dwMax) return FALSE;
  ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

  ofn.lStructSize = sizeof(OPENFILENAMEW);
  ofn.hwndOwner = hOwner;
  ofn.lpstrFilter = L"所有文件(*.*)\0*.*\0\0";
  ofn.lpstrFile = lpFilePath;
  ofn.nMaxFile = dwMax;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
  return GetOpenFileNameW(&ofn);
}

BOOL GetSaveFilePath(HWND hOwner, LPWSTR lpFilePath, DWORD dwMax)
{
  OPENFILENAMEW ofn;

  if (!lpFilePath || !dwMax) return FALSE;
  ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

  ofn.lStructSize = sizeof(OPENFILENAMEW);
  ofn.hwndOwner = hOwner;
  ofn.lpstrFilter = L"所有文件(*.*)\0*.*\0\0";
  ofn.lpstrFile = lpFilePath;
  ofn.nMaxFile = dwMax;
  ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
  return GetSaveFileNameW(&ofn);
}

BOOL ExportToFile(LPCWSTR lpFile, LPCVOID lpData, DWORD dwSize)
{
  HANDLE hFile = CreateFileW(lpFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  DWORD dwTmp;

  if (hFile == INVALID_HANDLE_VALUE) return FALSE;

  SetFilePointer(hFile, dwSize, NULL, FILE_BEGIN);
  SetEndOfFile(hFile);

  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
  WriteFile(hFile, lpData, dwSize, &dwTmp, NULL);

  CloseHandle(hFile);
  return TRUE;
}

BOOL ImportFromFile(LPCWSTR lpFile, LPVOID *lppData, DWORD *lpdwSize)
{
  HANDLE hFile = CreateFileW(lpFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  LPVOID lpData;
  DWORD dwSize, dwSizeHigh;

  if (hFile == INVALID_HANDLE_VALUE) return FALSE;

  dwSize = GetFileSize(hFile, &dwSizeHigh);
  if (dwSizeHigh != 0)
  {
    CloseHandle(hFile);
    return FALSE;
  }

  lpData = malloc(dwSize);
  if (lpData == NULL)
  {
    CloseHandle(hFile);
    return FALSE;
  }

  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
  ReadFile(hFile, lpData, dwSize, lpdwSize, NULL);

  *lppData = lpData;

  //free(lpData)
  CloseHandle(hFile);
  return TRUE;
}

void CleanListView()
{
  ListView_DeleteAllItems(GetDlgItem(hMainDlg, IDC_LV));
  while (ListView_DeleteColumn(GetDlgItem(hMainDlg, IDC_LV), 0));
}

void CleanView()
{
  TreeView_DeleteAllItems(GetDlgItem(hMainDlg, IDC_TV));
  CleanListView();
}

int UpdateView()
{
  HTREEITEM htiLastItem;
  TVINSERTSTRUCTW tvis;

  if (HWNP_GetState() != -1) return -1;
  CleanView();

  ZeroMemory(&tvis, sizeof(TVINSERTSTRUCTW));
  tvis.hParent = TVI_ROOT;
  tvis.hInsertAfter = TVI_LAST;
  tvis.item.mask = TVIF_TEXT | TVIF_PARAM;

  tvis.item.pszText = _T("<固件文件头部>");
  tvis.item.lParam = (LPARAM)TT_FILEHDR;
  htiLastItem = TreeView_DlgInsertItemW(hMainDlg, IDC_TV, &tvis);

  tvis.item.pszText = _T("<型号支持信息>");
  tvis.item.lParam = (LPARAM)TT_MODELINFO;
  htiLastItem = TreeView_DlgInsertItemW(hMainDlg, IDC_TV, &tvis);

  tvis.item.pszText = _T("<包含项目信息>");
  tvis.item.lParam = (LPARAM)TT_ITEMINFO;
  htiLastItem = TreeView_DlgInsertItemW(hMainDlg, IDC_TV, &tvis);

  TreeView_DlgSetItemW(hMainDlg, IDC_TV, htiLastItem);

  return 0;
}

void OpenFirmware(LPCWSTR lpPath)
{
  int nResult;

  if ((nResult = HWNP_OpenFirmware(lpPath)) != 0)
  {
    SetStatus(L"打开文件失败!错误码:[%d]", nResult);
    return;
  }

  if ((nResult = UpdateView()) != 0)
  {
    SetStatus(L"更新视图失败!错误码:[%d]", nResult);
    return ;
  }
}

