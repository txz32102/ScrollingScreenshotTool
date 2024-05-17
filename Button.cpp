#include "Button.h"

void Button::Create(HWND hwndParent, HINSTANCE hInstance, int x, int y, int width, int height, int id, const wchar_t* text) {
    CreateWindowEx(
        0,
        L"BUTTON",
        text,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, width, height,
        hwndParent,
        (HMENU)id,
        hInstance,
        NULL);
}
