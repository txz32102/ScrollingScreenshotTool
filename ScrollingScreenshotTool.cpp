#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Gdi32.lib")
#pragma comment (lib,"User32.lib")
#pragma comment (lib,"Ole32.lib")

using namespace Gdiplus;

const wchar_t g_szClassName[] = L"myWindowClass";
ULONG_PTR gdiplusToken;
HWND hEdit;  // 输入框句柄

// 初始化 GDI+
void InitGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    Status status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    if (status != Ok) {
        std::wcerr << L"Failed to initialize GDI+" << std::endl;
        exit(1);
    }
}

// 关闭 GDI+
void ShutdownGDIPlus() {
    GdiplusShutdown(gdiplusToken);
}

// 获取编码器 CLSID
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;          // 编码器数量
    UINT size = 0;         // 存储编码器信息的大小

    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;  // 没有编码器

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;  // 内存分配失败

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // 成功返回下标
        }
    }

    free(pImageCodecInfo);
    return -1;  // 没有找到
}

// 保存截图的函数
void SaveBitmapToFile(HBITMAP hBitmap, const WCHAR* filePath) {
    // 从 HBITMAP 创建 GDI+ Bitmap
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

    // 保存到文件
    Status status = bitmap.Save(filePath, &clsid, nullptr);
    if (status != Ok) {
        std::wcerr << L"Failed to save bitmap to file" << std::endl;
    }
    else {
        std::wcout << L"Screenshot saved as " << filePath << std::endl;
    }
}

// 截屏函数
void CaptureScreenshot() {
    // 获取屏幕尺寸
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 创建整个屏幕的设备上下文
    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // 创建一个与屏幕设备上下文兼容的位图
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    if (!hBitmap) {
        std::wcerr << L"Failed to create compatible bitmap" << std::endl;
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return;
    }

    // 将新位图选择到内存设备上下文中
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // 进行位块传输到我们的兼容内存 DC 中
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    // 恢复旧位图
    SelectObject(hMemoryDC, hOldBitmap);

    // 保存位图到文件
    SaveBitmapToFile(hBitmap, L"screenshot.png");

    // 清理
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);
}

// 延迟指定秒数
void Delay(int seconds) {
    Sleep(seconds * 1000);  // 将秒数转换为毫秒
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CreateWindow(
            L"BUTTON",  // 预定义类；假定为 Unicode 
            L"Click Me",      // 按钮文本 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // 样式 
            100,         // x 位置 
            100,         // y 位置 
            100,        // 按钮宽度
            30,        // 按钮高度
            hwnd,       // 父窗口
            (HMENU)1,       // 无菜单。
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL);      // 无需指针。

        // 创建输入框
        hEdit = CreateWindow(
            L"EDIT",   // 预定义类
            L"",       // 初始文本
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, // 样式
            100, 50, 100, 25,   // 位置和大小
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    }
    break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1)
        {
            // 获取用户输入的延迟时间
            wchar_t buffer[10];
            GetWindowText(hEdit, buffer, 10);
            int delaySeconds = _wtoi(buffer);  // 将文本转换为整数

            // 延迟指定的秒数
            Delay(delaySeconds);

            // 截屏
            CaptureScreenshot();
            MessageBox(hwnd, L"Screenshot saved as screenshot.png", L"Message", MB_OK);
        }
        break;
    case WM_CLOSE:
        // 在这里处理 WM_CLOSE 消息以防止窗口关闭
        if (MessageBox(hwnd, L"Are you sure you want to close?", L"Close", MB_OKCANCEL) == IDOK)
        {
            DestroyWindow(hwnd);
        }
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

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
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

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        ShutdownGDIPlus();
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    ShutdownGDIPlus();
    return (int)Msg.wParam;
}
