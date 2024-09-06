#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <vector>
#include <string>
#include "resource.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")

// Global variables
HWND g_hMainWindow = NULL;
HWND g_hListBox = NULL;
HWND g_hPiPWindow = NULL;
std::vector<HWND> g_windows;
HWND g_hTargetWindow = NULL;
HWND g_hExitButton = NULL;
HFONT g_hFont = NULL;

// Forward declarations
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PiPWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Populate the list box with window titles
void PopulateWindowList() {
    SendMessage(g_hListBox, LB_RESETCONTENT, 0, 0);
    g_windows.clear();

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (IsWindowVisible(hwnd) && GetParent(hwnd) == NULL) {
            wchar_t title[256];
            GetWindowTextW(hwnd, title, 256);
            if (wcslen(title) > 0) {
                SendMessageW(g_hListBox, LB_ADDSTRING, 0, (LPARAM)title);
                g_windows.push_back(hwnd);
            }
        }
        return TRUE;
    }, 0);
}

// Create the PiP window
void CreatePiPWindow(HWND targetWindow) {
    g_hTargetWindow = targetWindow;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int pipWidth = screenWidth / 4;
    int pipHeight = pipWidth * 9 / 16; // 16:9 aspect ratio

    int pipX = screenWidth - pipWidth - 20;
    int pipY = screenHeight - pipHeight - 40;

    g_hPiPWindow = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"PiPWindowClass",
        L"Stupid Pipview",
        WS_POPUP | WS_VISIBLE | WS_THICKFRAME,
        pipX, pipY, pipWidth, pipHeight,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (g_hPiPWindow == NULL) {
        MessageBox(NULL, L"Failed to create PiP window.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    g_hExitButton = CreateWindowEx(0, L"BUTTON", L"×",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        pipWidth - 20, 0, 20, 20, g_hPiPWindow, (HMENU)1, GetModuleHandle(NULL), NULL);

    ShowWindow(g_hPiPWindow, SW_SHOW);
    UpdateWindow(g_hPiPWindow);
}

// Update the PiP window content
void UpdatePiPWindow() {
    if (g_hTargetWindow && g_hPiPWindow && IsWindow(g_hTargetWindow)) {
        RECT rcTarget, rcPiP;
        GetWindowRect(g_hTargetWindow, &rcTarget);
        GetClientRect(g_hPiPWindow, &rcPiP);

        HDC hdcPiP = GetDC(g_hPiPWindow);
        HDC hdcMem = CreateCompatibleDC(hdcPiP);
        
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcPiP, rcTarget.right - rcTarget.left, rcTarget.bottom - rcTarget.top);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

        PrintWindow(g_hTargetWindow, hdcMem, PW_RENDERFULLCONTENT);

        SetStretchBltMode(hdcPiP, HALFTONE);
        StretchBlt(hdcPiP, 0, 0, rcPiP.right, rcPiP.bottom,
                   hdcMem, 0, 0, rcTarget.right - rcTarget.left, rcTarget.bottom - rcTarget.top, SRCCOPY);

        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(g_hPiPWindow, hdcPiP);
    } else if (g_hPiPWindow && !IsWindow(g_hTargetWindow)) {
        DestroyWindow(g_hPiPWindow);
    }
}

// Window procedure for the main window
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
        {
            g_hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                 OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            g_hListBox = CreateWindowEx(0, L"LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                10, 10, 300, 200, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
            SendMessage(g_hListBox, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            HWND hRefreshButton = CreateWindowEx(0, L"BUTTON", L"Refresh",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 220, 80, 30, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
            SendMessage(hRefreshButton, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            HWND hCreatePiPButton = CreateWindowEx(0, L"BUTTON", L"Create PiP",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                100, 220, 80, 30, hwnd, (HMENU)3, GetModuleHandle(NULL), NULL);
            SendMessage(hCreatePiPButton, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            HWND hCloseButton = CreateWindowEx(0, L"BUTTON", L"Close",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                190, 220, 80, 30, hwnd, (HMENU)4, GetModuleHandle(NULL), NULL);
            SendMessage(hCloseButton, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            PopulateWindowList();

            // the icon
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    if (!hIcon) {
        hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
    }
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

            return 0;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
                case 2: // Refresh button
                    PopulateWindowList();
                    break;
                case 3: // Create PiP button
                {
                    int selectedIndex = SendMessage(g_hListBox, LB_GETCURSEL, 0, 0);
                    if (selectedIndex != LB_ERR && selectedIndex < (int)g_windows.size()) {
                        HWND targetWindow = g_windows[selectedIndex];
                        CreatePiPWindow(targetWindow);
                    }
                    break;
                }
                case 4: // Close button
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }

        case WM_CTLCOLORLISTBOX:
        {
            HDC hdcListBox = (HDC)wParam;
            SetBkMode(hdcListBox, TRANSPARENT);
            SetTextColor(hdcListBox, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        case WM_DESTROY:
            if (g_hPiPWindow) {
                DestroyWindow(g_hPiPWindow);
            }
            DeleteObject(g_hFont);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Window procedure for the PiP window
LRESULT CALLBACK PiPWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            UpdatePiPWindow();
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_NCHITTEST:
        {
            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) hit = HTCAPTION;
            return hit;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == 1) { // Exit button
                DestroyWindow(hwnd);
            }
            return 0;
        }

        case WM_LBUTTONDBLCLK:
        {
            ShowWindow(g_hTargetWindow, SW_RESTORE);
            SetForegroundWindow(g_hTargetWindow);
            return 0;
        }

        case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            SetWindowPos(g_hExitButton, NULL, width - 20, 0, 20, 20, SWP_NOZORDER);
            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            g_hPiPWindow = NULL;
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Timer procedure to update PiP content
void CALLBACK UpdatePiPTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    if (g_hPiPWindow && IsWindow(g_hPiPWindow)) {
        InvalidateRect(g_hPiPWindow, NULL, FALSE);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindowProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"StupidPipviewClass";

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, L"Failed to register main window class.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    WNDCLASSEX wcexPiP = {0};
    wcexPiP.cbSize = sizeof(WNDCLASSEX);
    wcexPiP.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcexPiP.lpfnWndProc = PiPWindowProc;
    wcexPiP.hInstance = hInstance;
    wcexPiP.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcexPiP.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcexPiP.lpszClassName = L"PiPWindowClass";

    if (!RegisterClassEx(&wcexPiP)) {
        MessageBox(NULL, L"Failed to register PiP window class.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_hMainWindow = CreateWindowEx(
        0,
        L"StupidPipviewClass",
        L"Stupid Pipview",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 330, 300,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hMainWindow) {
        MessageBox(NULL, L"Failed to create main window.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hMainWindow, nCmdShow);
    UpdateWindow(g_hMainWindow);

    SetTimer(NULL, 0, 16, UpdatePiPTimerProc);  // Update at ~60 FPS

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}