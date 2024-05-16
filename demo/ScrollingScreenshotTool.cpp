#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <fstream>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Gdi32.lib")
#pragma comment (lib,"User32.lib")
#pragma comment (lib,"Ole32.lib")

using namespace Gdiplus;

const char g_szClassName[] = "myWindowClass";

// Function to save the screenshot
bool SaveBitmapToFile(HBITMAP hBitmap, const WCHAR* filePath) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // Initialize GDI+.
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok) {
        std::cerr << "Failed to initialize GDI+." << std::endl;
        return false;
    }

    // Create GDI+ Bitmap from HBITMAP
    Bitmap bitmap(hBitmap, nullptr);

    CLSID clsid;
    if (CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &clsid) != S_OK) {
        std::cerr << "Failed to get CLSID for BMP encoder." << std::endl;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Save to file
    if (bitmap.Save(filePath, &clsid, nullptr) != Ok) {
        std::cerr << "Failed to save bitmap to file." << std::endl;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Cleanup GDI+
    GdiplusShutdown(gdiplusToken);
    return true;
}

// Function to capture the screenshot
bool CaptureScreenshot() {
    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a device context for the entire screen
    HDC hScreenDC = GetDC(nullptr);
    if (!hScreenDC) {
        std::cerr << "Failed to get screen DC." << std::endl;
        return false;
    }

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        std::cerr << "Failed to create memory DC." << std::endl;
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Create a bitmap compatible with the screen device context
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    if (!hBitmap) {
        std::cerr << "Failed to create compatible bitmap." << std::endl;
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Select the new bitmap into the memory device context
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    if (!hOldBitmap) {
        std::cerr << "Failed to select bitmap into memory DC." << std::endl;
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Bit block transfer into our compatible memory DC
    if (!BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY)) {
        std::cerr << "BitBlt failed." << std::endl;
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Restore the old bitmap
    SelectObject(hMemoryDC, hOldBitmap);

    // Save the bitmap to a file
    bool saveResult = SaveBitmapToFile(hBitmap, L"screenshot.bmp");

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    return saveResult;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CreateWindow(
            "BUTTON",  // Predefined class; Unicode assumed 
            "Click Me",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            100,         // x position 
            100,         // y position 
            100,        // Button width
            30,        // Button height
            hwnd,       // Parent window
            (HMENU)1,       // No menu.
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.
    }
    break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1)
        {
            bool result = CaptureScreenshot();
            MessageBox(hwnd, "Screenshot saved as screenshot.bmp", "Message", MB_OK);
            if (result) {
                MessageBox(hwnd, "Screenshot saved as screenshot.bmp", "Message", MB_OK);
            }
            else {
                MessageBox(hwnd, "Failed to save screenshot.", "Error", MB_OK | MB_ICONERROR);
            }
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "homework",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
