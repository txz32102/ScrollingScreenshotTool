#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#include "Button.h"
#include <Shlwapi.h>

#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shlwapi.lib")


using namespace Gdiplus;

// Custom Point structure to avoid conflict with Gdiplus::Point
struct MyPoint {
    int x;
    int y;
};


// Global variables
const wchar_t g_szClassName[] = L"myWindowClass";
const wchar_t g_szOverlayClassName[] = L"myOverlayClass";
ULONG_PTR gdiplusToken;
MyPoint startPoint, endPoint;
bool capturing = false;
HWND hOverlayWnd;
HDC hdcScreen = NULL;
HDC hdcMemDC = NULL;
HBITMAP hBitmap = NULL;

// Initialize GDI+
void InitGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    if (status != Ok) {
        std::wcerr << L"Failed to initialize GDI+" << std::endl;
        exit(1);
    }
}

// Shutdown GDI+
void ShutdownGDIPlus() {
    GdiplusShutdown(gdiplusToken);
}

// Get encoder CLSID
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

// Save bitmap to file
void SaveBitmapToFile(HBITMAP hBitmap, const WCHAR* filePath) {
    Bitmap bitmap(hBitmap, nullptr);
    if (bitmap.GetLastStatus() != Ok) {
        std::wcerr << L"Failed to create GDI+ Bitmap" << std::endl;
        return;
    }

    CLSID clsid;
    if (GetEncoderClsid(L"image/png", &clsid) == -1) {
        std::wcerr << L"Failed to get PNG encoder CLSID" << std::endl;
        return;
    }

    Status status = bitmap.Save(filePath, &clsid, nullptr);
    if (status != Ok) {
        std::wcerr << L"Failed to save bitmap to file" << std::endl;
    }
    else {
        std::wcout << L"Screenshot saved as " << filePath << std::endl;

        // Open the saved screenshot
        ShellExecute(NULL, L"open", filePath, NULL, NULL, SW_SHOWNORMAL);
    }
}

// Capture screenshot
void CaptureScreenshot() {
    int width = endPoint.x - startPoint.x;
    int height = endPoint.y - startPoint.y;

    if (hdcScreen == NULL) {
        hdcScreen = GetDC(NULL);
        hdcMemDC = CreateCompatibleDC(hdcScreen);
        hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
        SelectObject(hdcMemDC, hBitmap);
    }

    BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, startPoint.x, startPoint.y, SRCCOPY);

    SaveBitmapToFile(hBitmap, L"screenshot.png");

    if (hOverlayWnd) {
        DestroyWindow(hOverlayWnd);
        hOverlayWnd = NULL;
    }
}

void CaptureScreen1920x1080() {
    // Get the screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate the width and height for a 1920x1080 screenshot
    int captureWidth = 1920;
    int captureHeight = 1080;

    // Calculate the starting point for the screenshot to capture the top-left corner
    int captureX = (screenWidth - captureWidth) / 2;
    int captureY = (screenHeight - captureHeight) / 2;

    // Create compatible DC and bitmap for capturing the screen
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, captureWidth, captureHeight);
    SelectObject(hdcMemDC, hBitmap);

    // Capture the screen
    BitBlt(hdcMemDC, 0, 0, captureWidth, captureHeight, hdcScreen, captureX, captureY, SRCCOPY);

    // Save the bitmap as BMP file
    SaveBitmapToFile(hBitmap, L"screenshot.bmp");

    // Clean up
    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
}


void FullWindow(const wchar_t* imagePath) {
    // Check if the file exists
    if (!PathFileExists(imagePath)) {
        std::cerr << "Error: Image file not found\n";
        return;
    }
    std::cout << "imagePath is loaded\n";

    // Register window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"FullWindowClass";

    RegisterClass(&wc);

    // Load the image
    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, imagePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hBitmap) {
        std::wcout << L"Image Path: " << imagePath << std::endl;
        std::cout << "error\n";
        std::cerr << "Error: Failed to load image\n";
        return;
    }

    // Create window
    HWND hWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        L"FullWindowClass",
        L"Full Window",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    std::cout << "hWnd is " << hWnd << std::endl;
    if (hWnd) {
        // Set window transparency
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
        // Set window background to the loaded image
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG)CreatePatternBrush(hBitmap));
        // Show window
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }
}


 //Mouse hook procedure
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;

        if (wParam == WM_LBUTTONDOWN) {
            startPoint.x = mouseStruct->pt.x;
            startPoint.y = mouseStruct->pt.y;
            capturing = true;
            std::cout << "Start Point: (" << startPoint.x << ", " << startPoint.y << ")\n";
        }
        else if (wParam == WM_MOUSEMOVE && capturing) {
            endPoint.x = mouseStruct->pt.x;
            endPoint.y = mouseStruct->pt.y;
            InvalidateRect(hOverlayWnd, NULL, TRUE);
        }
        else if (wParam == WM_LBUTTONUP && capturing) {
            endPoint.x = mouseStruct->pt.x;
            endPoint.y = mouseStruct->pt.y;
            capturing = false;
            std::cout << "End Point: (" << endPoint.x << ", " << endPoint.y << ")\n";
            PostQuitMessage(0); // Exit message loop
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Set semi-transparent brush
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));//����Ϊ��ɫ
        SetBkMode(hdc, TRANSPARENT);
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Draw the selected region without fill
        if (capturing) {
            RECT rect;
            rect.left = min(startPoint.x, endPoint.x);
            rect.top = min(startPoint.y, endPoint.y);
            rect.right = max(startPoint.x, endPoint.x);
            rect.bottom = max(startPoint.y, endPoint.y);
            FrameRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
        }

        DeleteObject(hBrush);
        EndPaint(hwnd, &ps);
    }
                 break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        PostMessage(GetParent(hwnd), msg, wParam, lParam);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        Button button1;
        button1.Create(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 100, 100, 100, 30, 1, L"Click Me");

        Button button2;
        button2.Create(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 220, 100, 100, 30, 2, L"New Button");

        Button button3;
        button2.Create(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 340, 100, 100, 30, 3, L"Debug");
    }
                  break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            // Create an overlay window
            hOverlayWnd = CreateWindowEx(
                WS_EX_LAYERED,
                g_szOverlayClassName,
                NULL,
                WS_POPUP,
                0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            if (hOverlayWnd) {
                SetLayeredWindowAttributes(hOverlayWnd, 0, 128, LWA_ALPHA); // Set transparency ����͸����Ϊ128
                ShowWindow(hOverlayWnd, SW_SHOW);
            }

            // Set mouse hook
            HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
            std::cout << "Move the mouse to the start point and click...\n";
            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            UnhookWindowsHookEx(mouseHook);

            CaptureScreenshot();
        }
        else if (LOWORD(wParam) == 2) {
            CaptureScreen1920x1080();
            MessageBox(hwnd, L"Hello World", L"Message", MB_OK);
            // This block handles the command for the control with ID 2
        }
        else if (LOWORD(wParam) == 3) {
            FullWindow(L"C:\\Users\\13816\\Desktop\\ScrollingScreenshotTool\\screenshot.bmp");
        }
        break;
    case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        if (hdcMemDC) DeleteDC(hdcMemDC);
        if (hBitmap) DeleteObject(hBitmap);
        if (hdcScreen) ReleaseDC(NULL, hdcScreen);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* consoleOut;
    freopen_s(&consoleOut, "CONOUT$", "w", stdout);
    InitGDIPlus();
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        ShutdownGDIPlus();
        return 0;
    }

    wc.lpszClassName = g_szOverlayClassName;
    wc.lpfnWndProc = OverlayWndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Overlay Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        ShutdownGDIPlus();
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"Screenshot Application",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 540, 200,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        ShutdownGDIPlus();
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }



    ShutdownGDIPlus();
    return (int)Msg.wParam;
}