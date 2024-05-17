#ifndef BUTTON_H
#define BUTTON_H

#include <windows.h>

class Button {
public:
    void Create(HWND hwndParent, HINSTANCE hInstance, int x, int y, int width, int height, int id, const wchar_t* text);
};

#endif
