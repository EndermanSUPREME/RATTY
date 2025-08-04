#ifndef DRAW
#define DRAW

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <random>

class RatArt {
public:
    RatArt(int id, int posX, int posY): id(id), x(posX), y(posY), text("~8>") {};
    bool isOffScreen(int width) const;
    
    std::string text;
    int id;
    int x;
    int y;
};

class Draw {
public:
    Draw(int count = 50): hdc(GetDC(NULL)), stopThread(false), ratCount(count) {};
    void CreateDrawSpace();
    void AnimateText(HWND hwnd, HDC hdcMem, HBITMAP hBitmap, void* pixels, int width, int height);
private:
    HDC hdc;
    int ratCount;
    bool stopThread;
    std::thread contentThread;
};

#endif