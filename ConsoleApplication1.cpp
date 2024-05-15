#include <windows.h>
#include <gdiplus.h>
#include <iostream>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Gdi32.lib")
#pragma comment (lib,"User32.lib")
#pragma comment (lib,"Ole32.lib")

using namespace Gdiplus;

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

int main() {
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
    return 0;
}
