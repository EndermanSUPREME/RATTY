#include <draw.hpp>

// generates a vector of random uniq shuffled numbers
std::vector<int> getUniqRand(int min, int max, int count) {
    std::vector<int> nums;
    for (int i = min; i <= max; ++i) nums.push_back(i);

    // shuffle reorders the elements in the given start|end range via RNG
    // std::mt19937 is a psuedo random RNG
    // std::random_device is a non-predictable RNG
    auto seed = std::random_device{}();
    auto rng = std::mt19937{seed};
    std::shuffle(nums.begin(), nums.end(), rng);

    nums.resize(count);  // Keep only the first `count` numbers
    return nums;
}

// generate random number based on range
int getRandom(int min, int max) {
    static std::random_device rd;  // Only used to seed the engine
    static std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

bool RatArt::isOffScreen(int width) const {
    return (x - 20) >= width;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        // run custom logic based on message id (uMsg)
        case WM_PAINT: // redraw the window
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        case WM_DESTROY: // end the window gracefully
            PostQuitMessage(0);
            return 0;
    }
    // handle the Window Handler (HWND hwnd) via a safe WinAPI standard
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Draw::AnimateText(HWND hwnd, HDC hdcMem, HBITMAP hBitmap, void* pixels, int width, int height) {
    // specifies how we want the bitmap to blend on the window
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    POINT ptZero = { 0, 0 };
    SIZE sizeWnd = { width, height };

    // store the current object to be deleted later (gets changed later)
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // creating monospace font for consistant text display
    HFONT hFont = CreateFontA(
        24,                // height
        0,                 // width (0 = default aspect ratio)
        0, 0,              // escapement & orientation
        FW_NORMAL,         // weight
        FALSE,             // italic
        FALSE,             // underline
        FALSE,             // strikeout
        ANSI_CHARSET,      // character set
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_MODERN,         // font family
        "Consolas"         // font name
    );
    // Apply the font to the HDC (display context)
    HFONT oldFont = (HFONT)SelectObject(hdcMem, hFont);

    // create rats in random positions
    std::vector<RatArt> swarm;
    std::vector<int> randXCollection = getUniqRand(10, width - 10, ratCount);
    std::vector<int> randYCollection = getUniqRand(10, height - 10, ratCount);

    for (int i = 0; i < ratCount; ++i) {
        int x = -randXCollection[i];
        swarm.emplace_back(i, x, randYCollection[i]);
    }

    while (!stopThread) {
        // Clears the pixel buffer to erase the previous frame
        ZeroMemory(pixels, width * height * 4);
        // Set text drawing options
        SetBkMode(hdcMem, TRANSPARENT); // transparent background
        SetTextColor(hdcMem, RGB(255, 0, 0));  // Red text

        for (RatArt& rat : swarm) {
            // Draw the text at (x, textY)
            TextOutA(hdcMem, rat.x, rat.y, rat.text.c_str(), (int)rat.text.length());
            if (rat.isOffScreen(width)) {
                // reset rats x position to random position
                // (give the illusion of endless rats)
                rat.x = -getRandom(10, width - 10);
            } else {
                // move across the screen
                ++rat.x;
            }
        }
        // Update the Window with new content (refresh)
        UpdateLayeredWindow(hwnd, hdc, NULL, &sizeWnd, hdcMem, &ptZero, 0, &blend, ULW_ALPHA);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // general clean up
    SelectObject(hdcMem, oldFont); // restore old font
    DeleteObject(hFont); // clean up new font
    SelectObject(hdcMem, oldBmp); // restore old bitmap
    ReleaseDC(NULL, hdc);
}

void Draw::CreateDrawSpace() {
    const char* CLASS_NAME = "OverlayWindow";

    // required for creating and registering the window class
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // defines the window class that all windows of this type will use
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH); // No background repaint

    // register the window class made above
    RegisterClass(&wc);

    // get screen meta-data
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // create the new window
    HWND hwnd = CreateWindowEx(
    // Allow UpdateLayeredWindow | Keeps the Window on Top | Mouse Clicks pass through the window
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        CLASS_NAME, "Overlay",
        WS_POPUP,
        0, 0, width, height,
        NULL, NULL, hInstance, NULL
    );

    // display it on the screen
    ShowWindow(hwnd, SW_SHOW);

    // Create 32-bit bitmap with alpha channel
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pixels = nullptr;
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);

    // clears the original frame and overwrites it with transparent pixels
    ZeroMemory(pixels, width * height * 4); // fully transparent

    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Setup blending
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    POINT ptZero = { 0, 0 };
    SIZE sizeWnd = { width, height };

    // Show fully transparent background (only what you draw on bitmap will show)
    UpdateLayeredWindow(hwnd, hdc, NULL, &sizeWnd, hdcMem, &ptZero, 0, &blend, ULW_ALPHA);

    // To Draw on top of this Window place code below this line!
    contentThread = std::thread(
        &Draw::AnimateText, this,
        hwnd,
        hdcMem,
        hBitmap,
        pixels,
        width,
        height
    );

    // Capture messages from active Window that get processed
    // by `WindowProc`
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // stop the thread and ensure it closes safely
    stopThread = true;
    if (contentThread.joinable()) {
        contentThread.join();
    }

    // Cleanup
    SelectObject(hdcMem, oldBmp);
    DeleteObject(hBitmap);

    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);
}