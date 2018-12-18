// FileExplorer.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "FileExplorer.h"

#define MAX_LOADSTRING MAX_PATH
#define ADDRESS_HEIGHT 40
#define STATUS_HEIGHT 36
#define LINE 3

// Global Variables:
static HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_hwnd;
HACCEL hAccelTable;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	CreateDir(HWND, UINT, WPARAM, LPARAM);
POINT				GetStartPoint(int, int);
VOID				GetInfoPC();
VOID				LoadTree(HWND);
VOID				LoadExpand(HWND, HTREEITEM);
INT					CheckChild(LPSHELLFOLDER, LPITEMIDLIST);
VOID				LoadListPC(HWND);
VOID				LoadList (HWND, LPCWSTR);
VOID				StatusBarItem();
LPWSTR				ConvertSize(unsigned long long size);
VOID				ColForFolder(HWND);
VOID				ColForDrive(HWND);
LPWSTR				GetDateModified(const FILETIME&);
LPCWSTR				GetPath(HWND, HTREEITEM);
INT					ItemEnter(HWND , LPCWSTR);
LPCWSTR				GetPath(HWND hWndListView, int iItem);
LPCWSTR				Return(LPCWSTR); 
LPWSTR				GetTypeFile(LPWSTR name);
WCHAR*				GetType(WIN32_FIND_DATA fd);
VOID				ChangeView(LONG style);
VOID				LoadIcon();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FILEEXPLORER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FILEEXPLORER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(g_hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }
    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FILEEXPLORER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(RGB(255,255,255));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FILEEXPLORER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_FILEEXPLORER));

    return RegisterClassExW(&wcex);

}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   POINT center = GetStartPoint(1000, 600);

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      center.x, center.y, 1000, 600, nullptr, nullptr, hInstance, nullptr);

   g_hwnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

struct Drive
{
	 WCHAR driver[26][4] = { 0 };
	 WCHAR drivetype[26][30] = { 0 };
	 WCHAR displayname[26][255] = { 0 };
	 int count = 0;
};

const INT BUFFERSIZE = MAX_PATH + 1;
WCHAR curPath[BUFFERSIZE];
WCHAR configPath[BUFFERSIZE];
WCHAR buffer[MAX_PATH + 1];
WCHAR address[MAX_PATH + 1] = L"This PC";
WCHAR adrCopy[MAX_PATH + 1] = { 0 };
WCHAR nameCopy[MAX_PATH + 1] = { 0 };
HWND hWndTreeView;
HWND hWndListView;
HWND lblAddress;
HWND hWndStatusBar;
Drive dr;
LOGFONT lf;
HICON s = NULL;
HANDLE ico;
HFONT font;
ACCEL * backup;
HIMAGELIST hmlbig;
HIMAGELIST hmlsmall;
LV_COLUMN lvCol;
WORD t;
int tvWidth;
int preWidth;
int height;
int width;
bool xSizing;
int bigIcon;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
	{
		height = HIWORD(lParam);
		width = LOWORD(lParam);
		RECT rect;
		GetWindowRect(hWnd, &rect);
		MoveWindow(GetDlgItem(hWnd, IDB_BACK), 3, 0, 40, 40, TRUE);
		MoveWindow(lblAddress, 46, 1, width - 50, 38, TRUE);
		SendMessage(hWndStatusBar, SB_SETMINHEIGHT, STATUS_HEIGHT, 0);
		MoveWindow(hWndStatusBar, 0, 0, 0, 0, TRUE);

		if (height < 200)
		{
			height = 200;
			MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, 200, TRUE);
		}
		if (width - tvWidth > 200)
		{
			MoveWindow(hWndTreeView, 0, ADDRESS_HEIGHT, tvWidth, height - ADDRESS_HEIGHT - STATUS_HEIGHT - 3, TRUE);
			MoveWindow(hWndListView, tvWidth + 2, ADDRESS_HEIGHT, width - tvWidth - 2, height - ADDRESS_HEIGHT - STATUS_HEIGHT - 3, TRUE);
			preWidth = tvWidth;
		}
		else
		{
			MoveWindow(hWndListView, width - 200 + 2, ADDRESS_HEIGHT, 202, height - ADDRESS_HEIGHT - STATUS_HEIGHT - 3, TRUE);
			MoveWindow(hWndTreeView, 0, ADDRESS_HEIGHT, width - 200, height - ADDRESS_HEIGHT - STATUS_HEIGHT - 3, TRUE);
			preWidth = width - 200;
		}
	}
	break;
	case WM_MOUSEMOVE:
	{
		int xPos;
		int yPos;
		xPos = (int)LOWORD(lParam);
		yPos = (int)HIWORD(lParam);

		if((xPos > preWidth - 5) && (xPos < preWidth + 5) && yPos > 30)
			SetCursor(LoadCursor(nullptr, MAKEINTRESOURCE(32644)));
		else
			SetCursor(LoadCursor(nullptr, IDC_ARROW));

		if (wParam == MK_LBUTTON)
		{
			if (xSizing)
			{
				SetCursor(LoadCursor(nullptr, MAKEINTRESOURCE(32644)));
				if (width - xPos > 200 && xPos > 100)
				{
					preWidth = xPos;
					MoveWindow(hWndTreeView, 0, ADDRESS_HEIGHT, preWidth, height - ADDRESS_HEIGHT - STATUS_HEIGHT - 3, 1);
					MoveWindow(hWndListView, preWidth + 2, ADDRESS_HEIGHT, width - preWidth - 2, height - ADDRESS_HEIGHT - STATUS_HEIGHT - 3, 1);
					tvWidth = preWidth;
				}
			}
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		int xPos;
		int yPos;
		xPos = (int)LOWORD(lParam);
		yPos = (int)HIWORD(lParam);
		if ((xPos > preWidth - 5) && (xPos < preWidth + 5) && yPos > 30)
		{
			xSizing = 1;
			SetCapture(hWnd);
			if (xSizing)
			{
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(32644)));
			}
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		xSizing = 0;
		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	break;
	case WM_CREATE:
	{
		InitCommonControls();
		GetCurrentDirectory(BUFFERSIZE, curPath);
		wsprintf(configPath, L"%s\\config.ini", curPath);
		GetPrivateProfileString(L"app", L"height", L"600", buffer, 10, configPath);
		height = _wtoi(buffer);
		GetPrivateProfileString(L"app", L"width", L"1000", buffer, 10, configPath);
		width = _wtoi(buffer);
		GetPrivateProfileString(L"app", L"tvWidth", L"300", buffer, 10, configPath);
		tvWidth = _wtoi(buffer);
		MoveWindow(hWnd, GetStartPoint(width, height).x, GetStartPoint(width, height).y, width, height, 1);
		lf.lfHeight = 24;
		lf.lfWidth = 11;
		wcscpy_s(lf.lfFaceName, L"Segoe UI");
		font = CreateFontIndirect(&lf);

		CreateWindow(L"button", L"", WS_CHILD | WS_VISIBLE | BS_ICON | BS_CENTER, 270 , 0, 40, 40, hWnd, (HMENU)IDB_BACK, hInst, NULL);
		ico = LoadImageW(hInst, MAKEINTRESOURCE(IDI_RETURN), IMAGE_ICON, 36, 36, LR_LOADTRANSPARENT);
		SendMessage(GetDlgItem(hWnd, IDB_BACK), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);

		lblAddress = CreateWindowEx(WS_EX_CONTROLPARENT, L"static", L"This PC", WS_VISIBLE | WS_CHILD  | SS_LEFT | SS_CENTER | SS_PATHELLIPSIS, 0, 0, 0, 0, hWnd, NULL, hInst, 0);
		SendMessage(lblAddress, WM_SETFONT, WPARAM(font), TRUE);

		hWndTreeView = CreateWindowEx(WS_EX_CLIENTEDGE,WC_TREEVIEW, L"", WS_CHILD | WS_VISIBLE  | TVS_HASLINES | TVS_EX_DOUBLEBUFFER | TVS_HASBUTTONS | WS_VSCROLL | WS_HSCROLL | TVS_LINESATROOT, 0, 0, 0, 0, hWnd, (HMENU)IDM_TREEVIEW, hInst, NULL);

		hWndListView = CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW, L"", WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | LVS_EX_DOUBLEBUFFER | LVS_REPORT, 0, 0, 0, 0, hWnd, (HMENU)IDM_LISTVIEW, hInst, 0);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT);

		hWndStatusBar = CreateWindowEx(WS_EX_CLIENTEDGE, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, 0, hInst, 0);
		int sizes[3] = { 200,400,-1};
		SendMessage(hWndStatusBar, SB_SETPARTS, 3, (LPARAM)&sizes);
		SendMessage(hWndStatusBar, SB_SETMINHEIGHT, 50, 0);

		LoadIcon();
		SendMessage(hWndTreeView, TVM_SETIMAGELIST, 0, (LPARAM)hmlsmall);
		ListView_SetImageList(hWndListView, hmlsmall, LVSIL_SMALL);
		ListView_SetImageList(hWndListView, hmlbig, LVSIL_NORMAL);
		GetInfoPC();
		LoadTree(hWndTreeView);
		LoadListPC(hWndListView);
		SetFocus(hWndListView);

	}
		break;
	case WM_CONTEXTMENU:
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);
		HMENU hMenu = CreatePopupMenu();
		InsertMenu(hMenu, 0, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_COPY, L"Copy");
		InsertMenu(hMenu, 1, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_PASTE, L"Paste");
		InsertMenu(hMenu, 2, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_SELECTALL, L"Select all");
		InsertMenu(hMenu, 3, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_DELETE, L"Delete");
		InsertMenu(hMenu, 4, MF_BYCOMMAND | MF_STRING | MF_ENABLED, ID_PROPERTY, L"Properties...");
		TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_TOPALIGN, xPos, yPos, 0, hWnd, NULL);
	}
		break;
	case WM_NOTIFY:
	{
		UINT code = ((LPNMHDR)lParam)->code;
		switch (code)
		{
		case TVN_ITEMEXPANDING:
			LoadExpand(hWndTreeView, ((LPNMTREEVIEW)(LPNMHDR)lParam)->itemNew.hItem);
			break;
		case TVN_SELCHANGED:
		{
			ListView_DeleteAllItems(hWndListView);
			LPCWSTR m_path = (LPCWSTR)((LPNMTREEVIEW)(LPNMHDR)lParam)->itemNew.lParam;
			LoadList(hWndListView, m_path);
			wcscpy_s(address, MAX_PATH, m_path);
			SendMessage(lblAddress, WM_SETTEXT, 0, (LPARAM)address);
			break;
		}
		case LVN_ITEMCHANGED:
		{
			WCHAR numitem[20];
			int num = ListView_GetSelectedCount(hWndListView);
			_itow_s(num, numitem, 10);
			if (num == 0)
			{
				SendMessage(hWndStatusBar, SB_SETTEXT, 1
					, 0);
				break;
			}
			else if (num > 1)
				wcscat_s(numitem, L" items selected");
			else
				wcscat_s(numitem, L" item selected");
			SendMessage(hWndStatusBar, SB_SETTEXT, 1, (LPARAM)numitem);
		}
			break;
		case NM_CLICK:
		{
			SendMessage(hWndStatusBar, SB_SETTEXT, 2, 0);
			if ((((LPNMHDR)lParam)->hwndFrom) != hWndListView)
			{
				for (int i = ListView_GetItemCount(hWndListView) - 1; i >= 0; --i)
				{
					ListView_SetItemState(hWndListView, i, 0,LVIS_SELECTED);
					ListView_SetSelectionMark(hWndListView, i);
				}
			}
			if ((((LPNMHDR)lParam)->hwndFrom) == hWndListView)
			{
				if (((LPNMITEMACTIVATE)lParam)->iItem == -1)
					ListView_SetSelectionMark(hWndListView, -1);
				else
				{
					LPCWSTR m_path = GetPath(hWndListView, ((LPNMITEMACTIVATE)lParam)->iItem);
					WIN32_FIND_DATA fd;
					GetFileAttributesEx(m_path, GetFileExInfoStandard, &fd);
					if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
					{
						SendMessage(hWndStatusBar, SB_SETTEXT, 2, (LPARAM)ConvertSize(fd.nFileSizeLow));
					}
					SendMessage(hWndStatusBar, SB_SETICON, 0, (LPARAM)ExtractAssociatedIconW(hInst, (LPWSTR)m_path, &t));

				}
			}
		}
		break;
		case NM_DBLCLK:
			if ((((LPNMHDR)lParam)->hwndFrom) == hWndListView)
			{
				if (((LPNMITEMACTIVATE)lParam)->iItem != -1)
				{
					LPCWSTR m_path = GetPath(hWndListView, ((LPNMITEMACTIVATE)lParam)->iItem);
					if(!ItemEnter(hWndListView, m_path))
						wcscpy_s(address, MAX_PATH, m_path);
					SendMessage(lblAddress, WM_SETTEXT, 0, (LPARAM)address);
				}
			}
			break;
		}
	}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case IDB_BACK:
				ListView_DeleteAllItems(hWndListView);
				LoadList(hWndListView, Return(address));
				SendMessage(lblAddress, WM_SETTEXT, 0, (LPARAM)address);
				break;
			case ID_ENTER:
				if (ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED) != -1)
				{
					LPCWSTR m_path = GetPath(hWndListView, ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED));
					if (!ItemEnter(hWndListView, m_path))
						wcscpy_s(address, MAX_PATH, m_path);
					SendMessage(lblAddress, WM_SETTEXT, 0, (LPARAM)address);
				}
				break;
			case ID_UP:
				SendMessage(hWnd, WM_COMMAND, IDB_BACK, 0);
				break;
			case ID_RELOAD:
				dr.count = 0;
				GetInfoPC();
				TreeView_DeleteAllItems(hWndTreeView);
				LoadTree(hWndTreeView);
				ListView_DeleteAllItems(hWndListView);
				LoadList(hWndListView, address);
				SetFocus(hWndListView);
				break;
			case ID_VIEW_LARGE:
				ChangeView(LVS_ICON);
				break;
			case ID_VIEW_SMALL:
				ChangeView(LVS_SMALLICON);
				break;
			case ID_VIEW_LIST:
				ChangeView(LVS_LIST);
				break;
			case ID_VIEW_REPORT:
				ChangeView(LVS_REPORT);
				break;
			case IDM_ABOUT:
				DialogBoxW(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case ID_COPY:
			{
				if (ListView_GetSelectionMark(hWndListView) != -1)
				{
					WCHAR m_path[MAX_PATH];
					wcscpy_s(m_path, GetPath(hWndListView, ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED)));
					ListView_GetItemText(hWndListView, ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED), 0, nameCopy, MAX_PATH);
					memset(adrCopy, 0, MAX_PATH);
					wcscpy_s(adrCopy, MAX_PATH, m_path);
				}
			}
				break;
			case ID_PASTE:
			{
				WCHAR m_path[MAX_PATH] = { 0 };
				if (ListView_GetSelectionMark(hWndListView) != -1)
				{
					wcscpy_s(m_path, GetPath(hWndListView, ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED)));
				}
				else
				{
					if (!wcscmp(address, L"This PC") || !wcscmp(adrCopy, L""))
						break;
					wcscpy_s(m_path, address);
				}
				if ((GetFileAttributes(m_path) & FILE_ATTRIBUTE_DIRECTORY) || ((GetFileAttributes(m_path) & FILE_ATTRIBUTE_DEVICE)))
				{
					if ((GetFileAttributes(adrCopy) & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcslen(m_path) != 3)
							wcscat_s(m_path, L"\\");
						wcscat_s(m_path, nameCopy);
						if (!wcscmp(m_path, adrCopy))
							wcscat_s(m_path, L"COPY");

						wcscat_s(adrCopy, L"\\*");
						adrCopy[wcslen(adrCopy) + 1] = 0;
					}
					else
					{
						adrCopy[wcslen(adrCopy) + 1] = 0;
					}

					m_path[wcslen(m_path) + 1] = 0;


					SHFILEOPSTRUCT sh;
					sh.hwnd = NULL;
					sh.pFrom = adrCopy;
					sh.pTo = m_path;
					sh.lpszProgressTitle = L"Copy";
					sh.wFunc = FO_COPY;
					sh.fFlags = FOF_RENAMEONCOLLISION | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMMKDIR;
					if (!SHFileOperation(&sh))
					{
						MessageBeep(0);
					}
					SendMessage(hWnd, WM_COMMAND, ID_RELOAD, 0);
				}
			}
			break;
			case ID_SELECTALL:
			{
				for (int i = ListView_GetItemCount(hWndListView) - 1; i >= 0; --i)
				{
					ListView_SetItemState(hWndListView, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				}
			}
				break;
			case ID_DELETE:
			{
				WCHAR m_path[MAX_PATH];
				SHFILEOPSTRUCT sh;
				sh.hwnd = NULL;
				sh.lpszProgressTitle = L"Delete";
				sh.wFunc = FO_DELETE;
				sh.fFlags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING | FOF_SIMPLEPROGRESS;
				int hoi = MessageBox(hWnd, L"Are you sure, that want to delete this item?", L"Warning!", MB_YESNOCANCEL | MB_ICONQUESTION);
				if (hoi == IDNO)
					sh.fFlags = (~FOF_ALLOWUNDO) & sh.fFlags;
				else if (hoi == IDCANCEL)
					break;
				int iPos = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);
				while (iPos != -1) {
					wcscpy_s(m_path, GetPath(hWndListView, iPos));
					m_path[wcslen(m_path) + 1] = 0;
					sh.pFrom = m_path;
					SHFileOperation(&sh);
					iPos = ListView_GetNextItem(hWndListView, iPos, LVNI_SELECTED);
				}
				SendMessage(hWnd, WM_COMMAND, ID_RELOAD, 0);
			}
				break;
			case ID_PROPERTY:
			{
				if (ListView_GetSelectionMark(hWndListView) != -1)
				{
					WCHAR m_path[MAX_PATH];
					wcscpy_s(m_path, GetPath(hWndListView, ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED)));
					ListView_GetItemText(hWndListView, ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED), 0, nameCopy, MAX_PATH);
					memset(adrCopy, 0, MAX_PATH);
					wcscpy_s(adrCopy, MAX_PATH, m_path);
					if (hWndListView)
					{
						SHELLEXECUTEINFO fileInfo;

						ZeroMemory(&fileInfo, sizeof(SHELLEXECUTEINFO));
						fileInfo.cbSize = sizeof(SHELLEXECUTEINFO);
						fileInfo.lpVerb = L"Properties";
						fileInfo.lpFile = adrCopy;
						fileInfo.nShow = SW_SHOW;
						fileInfo.fMask = SEE_MASK_INVOKEIDLIST;
						ShellExecuteEx(&fileInfo);
					}
				}
			}
				break;
			case ID_FILE_CREATENEWDIRECTORY:
			{
				DialogBoxW(hInst, MAKEINTRESOURCE(IDD_CREATEDIRECTOTY), hWnd, CreateDir);
				SendMessage(hWnd, WM_COMMAND, ID_RELOAD, 0);
			}
				break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
	{
		RECT rect;
		GetWindowRect(hWnd, &rect);
		_itow_s(rect.bottom - rect.top, buffer, 10);
		WritePrivateProfileString(L"app", L"height", buffer, configPath);
		_itow_s(rect.right - rect.left, buffer, 10);
		WritePrivateProfileString(L"app", L"width", buffer, configPath);
		_itow_s(tvWidth, buffer, 10);
		WritePrivateProfileString(L"app", L"tvWidth", buffer, configPath);
		PostQuitMessage(0);
	}
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK CreateDir(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			WCHAR to[MAX_PATH], buf[MAX_PATH];

			buf[GetWindowText(GetDlgItem(hDlg, IDC_DIRECTORYNAME), buf, MAX_PATH)] = 0;

			wcscpy_s(to, address);
			wcscat_s(to, buf);
			switch (CreateDirectoryW(to, NULL))
			{
			case 1:
				MessageBoxW(g_hwnd, L"Directory created!", L"Warning!", MB_OKCANCEL | MB_ICONWARNING);
				break;
			case 0:
			{
				DWORD erCode = GetLastError();
				switch (erCode)
				{
				case ERROR_ALREADY_EXISTS:
				{
					MessageBoxW(g_hwnd, L"This directory already exists!\nEnter other name!!!", L"Warning!", MB_OKCANCEL | MB_ICONWARNING);
					SendMessage(g_hwnd, WM_COMMAND, ID_FILE_CREATENEWDIRECTORY, 0);
				}
				break;
				case ERROR_PATH_NOT_FOUND:
				{
					MessageBoxW(g_hwnd, L"Path not found!!!\nCheck Path!", L"Warning!", MB_OKCANCEL | MB_ICONWARNING);
					SendMessage(g_hwnd, WM_COMMAND, ID_FILE_CREATENEWDIRECTORY, 0);
				}
				break;
				}
			}
			break;
			}
			EndDialog(hDlg, LOWORD(0));
			return (INT_PTR)TRUE;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(0));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

LPCWSTR GetPath(HWND hWndTV,HTREEITEM hItem)
{
	TVITEM tv;
	tv.mask = TVIF_PARAM;
	tv.hItem = hItem;
	TreeView_GetItem(hWndTV, &tv);
	return (LPCWSTR)tv.lParam;
}

VOID LoadExpand(HWND hWndTreeView, HTREEITEM hParent)
{
	HTREEITEM child = TreeView_GetChild(hWndTreeView, hParent);
	if (child != NULL)
		return;
	WCHAR path[MAX_PATH];
	wcscpy_s(path, GetPath(hWndTreeView, hParent));
	if (path == NULL)
		return;
	LPSHELLFOLDER psFolder = NULL;
	LPENUMIDLIST penumID = NULL;
	LPITEMIDLIST pidl = NULL;
	HRESULT hr = NULL;
	HICON hIc = NULL;
	LPITEMIDLIST itembandau = ILCreateFromPath(path);
	WIN32_FIND_DATA data;

	SHBindToObject(NULL, itembandau, NULL, IID_IShellFolder, (void**)&psFolder);
	psFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &penumID);
	do
	{
		hr = penumID->Next(1, &pidl, NULL);
		if (hr != S_OK)
			break;
		WCHAR chuoitenfile[MAX_PATH];
		STRRET name;
		psFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &name);
		StrRetToBuf(&name, pidl, chuoitenfile, MAX_PATH);
		SHGetDataFromIDListW(psFolder, pidl, SHGDFIL_FINDDATA, &data, sizeof(WIN32_FIND_DATA));
		WCHAR * buffer = new WCHAR[wcslen(path) + wcslen(data.cFileName) + 2];
		wcscpy_s(buffer, wcslen(path) + wcslen(data.cFileName) + 2, path);
		if(wcslen(buffer) != 3)
			wcscat_s(buffer, wcslen(path) + wcslen(data.cFileName) + 2,L"\\");
		wcscat_s(buffer, wcslen(path) + wcslen(data.cFileName) + 2, data.cFileName);
		TV_INSERTSTRUCT tv;
		tv.hParent = hParent;
		tv.hInsertAfter = TVI_LAST;
		tv.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM |TVIF_CHILDREN;
		hIc = ExtractAssociatedIconW(hInst, (LPWSTR)buffer, &t);
		tv.item.iImage = ImageList_ReplaceIcon(hmlsmall, -1, hIc);
		tv.item.iSelectedImage = 1;
		tv.item.pszText = data.cFileName;
		tv.item.lParam = (LPARAM)buffer;
		tv.item.cChildren = 0;
		if (CheckChild(psFolder, pidl))
			tv.item.cChildren = 1;
		HTREEITEM itnew = TreeView_InsertItem(hWndTreeView, &tv);
		DestroyIcon(hIc);
	} while (hr == S_OK);	
	psFolder->Release();
	penumID->Release();
}

POINT GetStartPoint(int x, int y)
{
	POINT center;
	RECT rect;
	HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &rect);
	center.x = (rect.right - x) / 2;
	center.y = (rect.bottom - y) / 2;
	return center;
}

LPWSTR ConvertSize(unsigned long long size)
{
	if (size == 0)
		return L"";

	int type = 0;
	double dsize = (double)size;
	while (dsize >= 1024)
	{
		dsize /= 1024;
		type++;
	}
	if (dsize == 0)
		dsize = 1;
	WCHAR * res = new WCHAR[15];
	if (dsize == 0)
		return L"";
	if(type > 1)
		swprintf_s(res,15, L"%.1f", dsize);
	else
		swprintf_s(res, 15, L"%.0f", dsize);

	switch (type)
	{
	case 0://Bytes
		wcscat_s(res,15, _T(" bytes"));
		break;
	case 1:
		wcscat_s(res,15,_T(" KB"));
		break;
	case 2:
		wcscat_s(res,15,_T(" MB"));
		break;
	case 3:
		wcscat_s(res,15, _T(" GB"));
		break;
	wcscat_s(res,15, _T(" TB"));
	}
	return res;
}

VOID ColForFolder(HWND hWndListView)
{
	lvCol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cx = 250;
	lvCol.pszText = _T("Name");
	ListView_InsertColumn(hWndListView,0,&lvCol);

	lvCol.cx = 80;
	lvCol.fmt = LVCFMT_RIGHT;
	lvCol.pszText = _T("Size");
	ListView_InsertColumn(hWndListView, 1, &lvCol);

	lvCol.cx = 220;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = _T("Type");
	ListView_InsertColumn(hWndListView, 2, &lvCol);


	lvCol.cx = 160;
	lvCol.pszText = _T("Date");
	ListView_InsertColumn(hWndListView, 3, &lvCol);

}

VOID ColForDrive(HWND hWndListView)
{

	lvCol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cx = 160;
	lvCol.pszText = _T("Name");
	ListView_InsertColumn(hWndListView, 0, &lvCol);

	lvCol.cx = 120;
	lvCol.pszText = _T("Size");
	ListView_InsertColumn(hWndListView, 2, &lvCol);

	lvCol.cx = 140;
	lvCol.pszText = _T("Type");
	ListView_InsertColumn(hWndListView, 1, &lvCol);


	lvCol.cx = 125;
	lvCol.pszText = _T("Free space");
	ListView_InsertColumn(hWndListView, 3, &lvCol);

}

VOID GetInfoPC()
{
	LPWSTR buffer = new WCHAR[MAX_PATH];

	GetLogicalDriveStringsW(MAX_PATH, buffer);

	for (int i = 0;!(buffer[i] == 0 && buffer[i + 1] == 0);i++)
		if (buffer[i] == 0)
		{
			dr.count++;
		}
	dr.count++;

	int m = 0;;
	for (int i = 0; m < dr.count; i += 4)
	{
		wcscpy_s(dr.driver[m], &buffer[i]);
		m++;
	}

	for (int i = 0; i < dr.count; i++)
	{

		int type = GetDriveType(dr.driver[i]);
		if (type == DRIVE_FIXED)
		{
			wcscpy_s(dr.drivetype[i], L"Local disk");
			if (GetVolumeInformationW(dr.driver[i], buffer, MAX_PATH + 1, NULL, NULL, NULL, NULL, MAX_PATH + 1))
			{
				wcscpy_s(dr.displayname[i], buffer);
			}
			else
			{
				wcscpy_s(dr.displayname[i], L"Локальный диск");
			}
			if (buffer[0] == 0)
				wcscat_s(dr.displayname[i], L"Local disk");
		}
		else if (type == DRIVE_CDROM)
		{
			wcscpy_s(dr.displayname[i], L"CD Drive");
			wcscpy_s(dr.drivetype[i], L"CD Drive");
		}
		else if (i > 1 && type == DRIVE_REMOVABLE)
		{
			GetVolumeInformation(dr.driver[i], buffer, 100, NULL, NULL, NULL, NULL, 0);
			wcscpy_s(dr.displayname[i], buffer);
			if (buffer[0] == 0)
				wcscat_s(dr.displayname[i], L"USB");
			wcscpy_s(dr.drivetype[i], L"Removeble disk");

		}
		wcscat_s(dr.displayname[i], L" (");
		wcsncat_s(dr.displayname[i], dr.driver[i], 1);
		wcscat_s(dr.displayname[i], L":)");
	}

}

VOID LoadTree(HWND hWndTreeView)
{
	WCHAR buffer[MAX_PATH];
	HTREEITEM it;
	TV_INSERTSTRUCT tv;
	tv.hParent = TVI_ROOT;
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tv.item.iImage = 0;
	tv.item.iSelectedImage = 0;
	tv.item.pszText = L"This PC";
	tv.item.lParam = (LPARAM)L"This PC";
	it = TreeView_InsertItem(hWndTreeView, &tv);

	for (int i = 0; i < dr.count;i++)
	{
		INT32 type = GetDriveType(dr.driver[i]);
		memset(buffer, 0, MAX_PATH);
		tv.hParent = it;
		tv.hInsertAfter = TVI_LAST;
		tv.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN;
		s = ExtractAssociatedIconW(hInst, dr.driver[i], &t);
		tv.item.iImage = ImageList_AddIcon(hmlsmall, s);
		tv.item.iSelectedImage = ImageList_AddIcon(hmlsmall, s);
		tv.item.pszText = dr.displayname[i];
		tv.item.lParam = (LPARAM)dr.driver[i];
		tv.item.cChildren = 0;
		if (type == DRIVE_FIXED || (type == DRIVE_REMOVABLE))
		{
			tv.item.cChildren = 1;
		}
		HTREEITEM it_child = TreeView_InsertItem(hWndTreeView, &tv);
		DestroyIcon(s);
	}
	TreeView_Expand(hWndTreeView, it, TVE_EXPAND);
}

VOID LoadListPC(HWND hWndListView)
{
	for (int i = 0; i < 4; i++)
	{
		ListView_DeleteColumn(hWndListView, 0);
	}
	ColForDrive(hWndListView);
	for (int i = 0; i < dr.count; i++)
	{
		LV_ITEM lv;
		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lv.iItem = i;
		lv.iImage = ImageList_AddIcon(hmlsmall, ExtractAssociatedIconW(hInst, dr.driver[i], &t));
		if (bigIcon)
		{
			lv.iImage = ImageList_AddIcon(hmlbig, ExtractAssociatedIconW(hInst, dr.driver[i], &t));
		}
		lv.iSubItem = 0;
		lv.pszText = dr.displayname[i];
		lv.lParam = (LPARAM)dr.driver[i];
		ListView_InsertItem(hWndListView, &lv);
		lv.mask = LVIF_TEXT;
		lv.iSubItem = 1;
		lv.pszText = dr.drivetype[i];
		ListView_SetItem(hWndListView, &lv);
		if (wcscmp(dr.drivetype[i], L"CD Drive"))
		{
			lv.iSubItem = 2;
			unsigned long long size;
			SHGetDiskFreeSpaceEx(dr.driver[i], NULL, (PULARGE_INTEGER)&size, NULL);
			lv.pszText = ConvertSize(size);
			ListView_SetItem(hWndListView, &lv);

			lv.iSubItem = 3;
			SHGetDiskFreeSpaceEx(dr.driver[i], NULL, NULL, (PULARGE_INTEGER)&size);
			lv.pszText = ConvertSize(size);
			ListView_SetItem(hWndListView, &lv);
		}
	}
	StatusBarItem();
}

VOID LoadList(HWND hWndListView, LPCWSTR path)
{
	ListView_DeleteAllItems(hWndListView);
	for (int i = 0; i < 4; i++)
	{
		ListView_DeleteColumn(hWndListView, 0);
	}
	if (path == NULL)
		return;
	if (wcscmp(path, L"This PC") == 0)
	{
		LoadListPC(hWndListView);
		return;
	}
	ColForFolder(hWndListView);
	WCHAR buffer[MAX_PATH];
	wcscpy_s(buffer, path);
	LPWSTR folderPath = new WCHAR[MAX_PATH];
	INT32 count = 0;
	LPSHELLFOLDER psFolder = NULL;
	LPENUMIDLIST penumID = NULL;
	LPITEMIDLIST pidl = NULL;
	HRESULT hr = NULL;
	HICON hIco = NULL;
	WIN32_FIND_DATA data;
	if (SHBindToObject(NULL, ILCreateFromPathW(path), NULL, IID_IShellFolder, (void**)&psFolder) == S_FALSE)
		return;
	psFolder->EnumObjects(hWndListView, 
		SHCONTF_FOLDERS |  
		SHCONTF_SHAREABLE | 
		SHCONTF_ENABLE_ASYNC | 
		SHCONTF_INCLUDEHIDDEN | 
		SHCONTF_NONFOLDERS, (IEnumIDList**)&penumID);
	if (penumID == NULL)
		return;
	do
	{
		CoTaskMemRealloc(pidl, sizeof(IShellFolder));
		hr = penumID->Next(1, &pidl, 0);
		if (hr != S_OK)
			break;
		STRRET name;
		psFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &name);
		SHGetDataFromIDListW(psFolder, pidl, SHGDFIL_FINDDATA, &data, sizeof(WIN32_FIND_DATA));
		folderPath = new WCHAR[wcslen(path) + wcslen(data.cFileName) + 10];
		wcscpy_s(folderPath, wcslen(path) + wcslen(data.cFileName) + 10, path);
		if (wcslen(path) != 3)
		{
			wcscat_s(folderPath, wcslen(path) + wcslen(data.cFileName) + 10, L"\\");
		}
		wcscat_s(folderPath, wcslen(path) + wcslen(data.cFileName) + 10, data.cFileName);
		LV_ITEM lv;
		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lv.iItem = count;
		if (!bigIcon)
		{
			hIco = ExtractAssociatedIconW(hInst, folderPath, &t);
			lv.iImage = ImageList_ReplaceIcon(hmlsmall, -1, hIco);
		}
		else
		{
			hIco = ExtractAssociatedIconW(hInst, folderPath, &t);
			lv.iImage = ImageList_ReplaceIcon(hmlbig, -1, hIco);
		}
		DestroyIcon(hIco);
		lv.iSubItem = 0;
		lv.pszText = data.cFileName;
		lv.lParam = (LPARAM)folderPath;
		ListView_InsertItem(hWndListView, &lv);
		ListView_SetItemText(hWndListView, count, 2, GetType(data));
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				ListView_SetItemText(hWndListView, count, 2, L"LINK");
			}
			else
			{
				ListView_SetItemText(hWndListView, count, 2, L"DIRECTORY");
			}
		}
		ListView_SetItemText(hWndListView, count, 1, ConvertSize(data.nFileSizeLow));
		ListView_SetItemText(hWndListView, count, 3, GetDateModified(data.ftLastWriteTime));
		count++;
	} while (hr == S_OK);
	psFolder->Release();
	penumID->Release();
	StatusBarItem();
}

LPWSTR GetDateModified(const FILETIME &ftLastWrite)
{
	SYSTEMTIME t;
	FileTimeToSystemTime(&ftLastWrite, &t);
	WCHAR *buffer = new WCHAR[2048];
	swprintf_s(buffer, 50, L"%d/%02d/%02d %02d:%02d %02s",
		t.wDay, t.wMonth, t.wYear, (t.wHour>12) ? (t.wHour%12) : t.wHour,
		t.wMinute, (t.wHour > 12) ? L"PM" : L"AM");
	return buffer;
}

LPCWSTR GetPath(HWND hWndListView, int iItem)
{
	LVITEM lv;
	lv.mask = LVIF_PARAM;
	lv.iItem = iItem;
	lv.iSubItem = 0;
	ListView_GetItem(hWndListView, &lv);
	return (LPCWSTR)lv.lParam;
}

INT32 ItemEnter(HWND hWndListView, LPCWSTR path)
{
	if (!path)
		return 0;
	WIN32_FIND_DATA fd;
	GetFileAttributesEx(path, GetFileExInfoStandard, &fd);
	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		ListView_DeleteAllItems(hWndListView);
		LoadList(hWndListView, path);
		return 0;
	}
	ShellExecute(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
	return 1;
}

LPCWSTR Return(LPCWSTR path)
{
	if (wcscmp(path, L"This PC") == 0)
		return path;
	INT64 len = wcslen(path);
	if (len <= 3)
	{
		swprintf_s((WCHAR*)path, 10, L"This PC");
		return path;
	}
	INT64 i;
	for (i = len - 1; i > 0 && path[i] != '\\';i--);
	if(i > 2)
		(WCHAR)path[i] = 0;
	else
		(WCHAR)path[i+1] = 0;
	return path;
}

LPWSTR GetTypeFile(LPWSTR name)
{
	INT64 i;
	INT64 len = wcslen(name);
	WCHAR* res = new WCHAR[255];
	for (i = len - 1; i >= 0 && name[i] != '.'; i--);
	if (i == -1)
		return L"";
	wcscpy_s(res, 255, &name[i + 1]);
	_wcsupr_s(res, 255);
	return res;
}

WCHAR* GetType(WIN32_FIND_DATA fd)
{
	WCHAR *dot = new WCHAR[255];
	INT64 vt = wcsrchr(fd.cFileName, L'.') - fd.cFileName;
	INT64 len = wcslen(fd.cFileName);
	if (vt < 0 || vt >= len)
		return L"UNKNOWN";
	else
		wcscpy_s(dot,255,GetTypeFile(fd.cFileName));
	WCHAR *tail = new WCHAR[len - vt + 1];
	for (int i = 0; i < len - vt; i++)
		tail[i] = fd.cFileName[vt + i];
	tail[len - vt] = 0;
	if (!_wcsicmp(tail, L".htm") || !_wcsicmp(tail, L".html") || !_wcsicmp(tail, L".URL"))
		return L"Web page";
	WCHAR pszOut[256];
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 256;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, tail, 0, KEY_READ, &hKey))
	{
		RegCloseKey(hKey);
		return dot;
	}
	if (RegQueryValueEx(hKey, NULL, NULL, &dwType, (PBYTE)pszOut, &dwSize))
	{
		RegCloseKey(hKey);
		return dot;
	}
	RegCloseKey(hKey);
	WCHAR *pszPath = new WCHAR[MAX_PATH];
	dwSize = 2000;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, pszOut, 0, KEY_READ, &hKey))
	{
		RegCloseKey(hKey);
		return dot;
	}

	if (RegQueryValueEx(hKey, NULL, NULL, &dwType, (PBYTE)pszPath, &dwSize))
	{
		RegCloseKey(hKey);
		return dot;
	}
	RegCloseKey(hKey);
	return pszPath;
}
//Load Icon
VOID LoadIcon()
{
	hmlbig = ImageList_Create(48, 48, ILC_MASK | ILC_COLOR32, 1, 1);
	hmlsmall = ImageList_Create(24, 24, ILC_MASK | ILC_COLOR32, 1, 1);

	s = ExtractIconW(hInst, L"%SystemRoot%\\system32\\shell32.dll", 15);
	ImageList_AddIcon(hmlsmall, s);
	s = ExtractIconW(hInst, L"%SystemRoot%\\system32\\shell32.dll", 4);
	ImageList_AddIcon(hmlsmall, s);
	s = ExtractIconW(hInst, L"%SystemRoot%\\system32\\shell32.dll", 4);
	ImageList_AddIcon(hmlbig, s);
	DestroyIcon(s);
}

VOID StatusBarItem()
{
	WCHAR numitem[MAX_PATH];
	int num = ListView_GetItemCount(hWndListView);
	_itow_s(num, numitem, MAX_PATH, 10);
	if (num > 1)
		wcscat_s(numitem, L" items");
	else
		wcscat_s(numitem, L" item");
	SendMessage(hWndStatusBar, SB_SETTEXT, 0, (LPARAM)numitem);
	SendMessage(hWndStatusBar, SB_SETTEXT, 1, 0);
	SendMessage(hWndStatusBar, SB_SETTEXT, 2, 0);
}

INT32 CheckChild(LPSHELLFOLDER father, LPITEMIDLIST root)
{
	LPSHELLFOLDER child = NULL;
	father->BindToObject(root, NULL, IID_IShellFolder, (void**)&child);
	if (child == NULL)
		return 0;
	LPENUMIDLIST enumList = NULL;
	HRESULT hr = child->EnumObjects(NULL, SHCONTF_FOLDERS, &enumList);
	if (enumList == NULL)
		return 0;
	LPITEMIDLIST pidl = NULL;
	hr = enumList->Next(1, &pidl, NULL);
	if (hr != S_OK)
		return 0;
	return 1;
}

VOID ChangeView(LONG style)
{
	LONG notchoose = ~(LVS_REPORT | LVS_LIST | LVS_ICON | LVS_SMALLICON);
	SetWindowLong(hWndListView, GWL_STYLE, GetWindowLong(hWndListView, GWL_STYLE)&notchoose | style);
	if (LVS_ICON == style)
		bigIcon = 1;
	else
		bigIcon = 0;
	ListView_DeleteAllItems(hWndListView);
	LoadList(hWndListView, address);
	SetFocus(hWndListView);
}