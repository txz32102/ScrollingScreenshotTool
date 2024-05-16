#include <windows.h>
#include <gdiplus.h>
#include <iostream>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Gdi32.lib")
#pragma comment (lib,"User32.lib")
#pragma comment (lib,"Ole32.lib")

using namespace Gdiplus;

const char g_szClassName[] = "myWindowClass";

// Function to save the screenshot
void SaveBitmapToFile(HBITMAP hBitmap, const WCHAR* filePath) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Create GDI+ Bitmap from HBITMAP
    Bitmap bitmap(hBitmap, nullptr);

    CLSID clsid;
    CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &clsid); // BMP encoder

    // Save to file
    bitmap.Save(filePath, &clsid, nullptr);

    // Cleanup GDI+
    GdiplusShutdown(gdiplusToken);
}

// Function to capture the screenshot
void CaptureScreenshot() {
    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a device context for the entire screen
    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Create a bitmap compatible with the screen device context
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);

    // Select the new bitmap into the memory device context
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Bit block transfer into our compatible memory DC
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    // Restore the old bitmap
    SelectObject(hMemoryDC, hOldBitmap);

    // Save the bitmap to a file
    SaveBitmapToFile(hBitmap, L"screenshot.bmp");

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    std::cout << "Screenshot saved as screenshot.bmp" << std::endl;
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
            CaptureScreenshot();
            MessageBox(hwnd, "Screenshot saved as screenshot.bmp", "Message", MB_OK);
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
        "大作业",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
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
