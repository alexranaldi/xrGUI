#pragma once


#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

#include "strsafe.h"
#include "windows.h"
#include "Shellapi.h"

namespace xrGUI{

// fwd declarations
class Window;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void store(std::shared_ptr<Window> w);
std::shared_ptr<Window> getWindowByHandle(HWND hTest);
std::shared_ptr<Window> getWindowById(HMENU h);
LRESULT handleWinMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// globals
std::unordered_map <HMENU, std::shared_ptr<Window>> windowMap;
int childId = 100;

struct WinColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    WinColor(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) 
    {}
    COLORREF toColorRef() const {
        COLORREF ret = 0;
        ret |= r;
        ret |= (static_cast<COLORREF>(g)<<8);
        ret |= (static_cast<COLORREF>(b) << 16);
        return ret;
    }
};

const auto RED = WinColor(255, 0, 0);
const auto L_RED_1 = WinColor(255, 68, 68);
const auto GREY_BLUE = WinColor(30, 45, 70);
const auto BLACK = WinColor(0, 0, 0);
const auto WHITE = WinColor(255, 255, 255);
const auto GREY_1 = WinColor(190, 190, 190);
const auto GREY_2 = WinColor(175, 175, 175);

using F_CALLBACK = std::function<void()>;
using F_RESIZE_CALLBACK = std::function<LRESULT(int, int)>;

struct XYWH {
    int x;
    int y;
    int w;
    int h;
};

static HMENU getNextId() {
    childId++;
    return (HMENU)childId;
}

class Window {
public:
    Window(HWND hPar) :
        hWnd(nullptr),
        hWndParent(hPar)
    {
        id = getNextId();
    }
    virtual bool onClose() {
        if (closeCallback) {
            closeCallback();
        }
        DestroyWindow(hWnd);
        return true;
    }
    virtual bool onDestroy() {
        if (destroyCallback) {
            destroyCallback();
        }
        return false; //return 0 after processing
    }
    virtual bool onMenuCommand(const int idx) {
        return true;
    }
    virtual bool onCommand(UINT message, WPARAM wParam, LPARAM lParam) {
        return false;
    }
    virtual bool onDraw(UINT message, WPARAM wParam, LPARAM lParam) {
        return false;
    }
    virtual bool setPosition(XYWH pos) {
        MoveWindow(hWnd, pos.x, pos.y, pos.w, pos.h, true);
        return true;
    }
    virtual bool show(int nCmdShow) {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
        return true;
    }
    virtual bool onMeasureItem(UINT message, WPARAM wParam, LPARAM lParam) {
        return false;
    }
    virtual LRESULT onColorStatic(UINT message, WPARAM wParam, LPARAM lParam) {
        return 0; // return brush, see WM_CTLCOLORSTATIC
    }
    virtual void setFont(const std::string& fontName, const long fontSize) {
        HDC hdc = GetDC(hWnd);
        LOGFONTA logFont = { 0 };
        logFont.lfHeight = -MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        logFont.lfWeight = FW_NORMAL;
        strcpy_s(logFont.lfFaceName, fontName.c_str());
        //ReleaseDC(hWnd, hdc);
        HFONT myFont = CreateFontIndirectA(&logFont);
        SendMessageA(hWnd, WM_SETFONT, WPARAM(myFont), TRUE);
    }
    virtual LRESULT onResize(int h, int w) {
        return resizeCallback(h,w);
    }
    void setResizeCallback(F_RESIZE_CALLBACK f) {
        resizeCallback = f;
    }
    void setCloseCallback(F_CALLBACK f) {
        closeCallback = f;
    }
    void setDestroyCallback(F_CALLBACK f) {
        destroyCallback = f;
    }
    HWND hWnd;
    HWND hWndParent;
    HMENU id;
    F_CALLBACK closeCallback;
    F_CALLBACK destroyCallback;
    F_RESIZE_CALLBACK resizeCallback;
};

class Clickable {
public:
    virtual void onClick() {
        if (clickCb != nullptr) {
            clickCb();
        }
    }
    virtual void setClickCallback(F_CALLBACK f) {
        clickCb = f;
    }
    F_CALLBACK clickCb;
};

class MainWindow : public Window {
public:
    MainWindow(HINSTANCE hInstance, const std::string& title, const std::string& wclass, HMENU hMenu) : Window(nullptr) {
        hWnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            title.c_str(), // title
            wclass.c_str(), // class
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1400,
            800,
            NULL,               // hWndParent
            hMenu,               // hmenu
            hInstance,
            NULL);
    }
    virtual ~MainWindow() {}
private:
};

struct LBString {
    std::string str;
    WinColor rgb_fg;
    WinColor rgb_bg;
    LBString(const char* s) :
        str(s), rgb_fg(BLACK), rgb_bg(WHITE)
    {}
    LBString(const std::string& s) :
        str(s), rgb_fg(BLACK), rgb_bg(WHITE)
    {}
    LBString(const std::string& s, const WinColor fgCol, const WinColor bgCol) :
        str(s), rgb_fg(fgCol), rgb_bg(bgCol)
    {}
};

class Menu : public Window {
public:
    std::unordered_map<int, F_CALLBACK> itemCBs;
    int nextItemId;
    Menu() : Window (NULL), nextItemId(0) {
        hWnd = (HWND)CreateMenu();
    }
    int addTextItem(const std::string& label) {
        int cid = nextItemId;
        nextItemId++;
        AppendMenuA((HMENU)hWnd, MF_STRING, cid, label.c_str());
        return cid;
    }
    void addSubMenu(std::shared_ptr<Menu> sub, const std::string& label) {
        AppendMenuA((HMENU)hWnd, MF_POPUP, (UINT_PTR)sub->hWnd, label.c_str());
    }
    void setClickCallback(int id, F_CALLBACK cb) {
        itemCBs[id] = cb;
    }
    void onClick(int x) {
        auto cb = itemCBs[x];
        cb();
    }
    bool onCommand(UINT message, WPARAM wParam, LPARAM lParam) {
        onClick((int)LOWORD(wParam));
        return true;
    }
    bool onMenuCommand(const int idx) override {
        if (itemCBs.find(idx) == itemCBs.end()) {
            return false;
        }
        itemCBs[idx]();
        return true;
    }
};

class OpenGLContext : public Window {
public:
    OpenGLContext(HWND hPar, HINSTANCE hInstance) : Window(hPar) {
        hWnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "Static",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            1,
            1,
            1,
            1,
            hPar,
            id,
            hInstance,
            NULL);
    }
};

class EditBox : public Window, public Clickable {
public:
    EditBox(HWND hPar, HINSTANCE hInstance) : Window(hPar) {
        hWnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "Edit",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER,
            1,
            1,
            1,
            1,
            hPar,
            id,
            hInstance,
            NULL);
    }
    void setText(const std::string& str) {
        SendMessageA(hWnd, WM_SETTEXT, NULL, (LPARAM)str.c_str());
    }
    bool onCommand(UINT message, WPARAM wParam, LPARAM lParam) override {
        onClick();
        return true;
    }
    std::string getText() {
        char buf[255] = {0};
        SendMessageA(hWnd, WM_GETTEXT, (WPARAM)255, (LPARAM)buf);
        return std::string(buf);
    }
};

class ComboBox : public Window, public Clickable {
public:
    std::vector<std::string> strings;
    ComboBox(HWND hPar, HINSTANCE hInstance) : Window(hPar) {
        hWnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "ComboBox",
            NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP,
            1,
            1,
            1,
            1,
            hPar,
            id,
            hInstance,
            NULL);
    }
    bool onCommand(UINT message, WPARAM wParam, LPARAM lParam) override {
        // combobox selection changed
        const int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
            (WPARAM)0, (LPARAM)0);
        onClick();
        return true;
    }
    void addString(const std::string& str) {
        SendMessageA(
            hWnd,
            CB_ADDSTRING,
            0,
            (LPARAM)str.c_str());
        strings.push_back(str);
    }
    bool onDraw(UINT message, WPARAM wParam, LPARAM lParam) override {
        COLORREF clrBackground;
        COLORREF clrForeground;
        TEXTMETRIC tm;
        int x;
        int y;
        HRESULT hr;
        size_t cch;
        CHAR achTemp[256];

        LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

        if (lpdis->itemID == -1) // Empty item)
            return true;

        // The colors depend on whether the item is selected.
        clrForeground = SetTextColor(lpdis->hDC,
            GetSysColor(lpdis->itemState & ODS_SELECTED ?
                COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        clrBackground = SetBkColor(lpdis->hDC,
            GetSysColor(lpdis->itemState & ODS_SELECTED ?
                COLOR_HIGHLIGHT : COLOR_WINDOW));

        GetTextMetrics(lpdis->hDC, &tm);

        // Get and display the text for the list item.
        SendMessageA(hWnd, CB_GETLBTEXT,
            lpdis->itemID, (LPARAM)achTemp);

        hr = StringCchLength(achTemp, 256, &cch);

        /*
        ExtTextOut(lpdis->hDC, 0, 0,
            ETO_CLIPPED | ETO_OPAQUE, &lpdis->rcItem,
            achTemp, (UINT)cch, NULL);
            */


        int yPos = (lpdis->rcItem.bottom + lpdis->rcItem.top -
            tm.tmHeight) / 2;
        TextOutA(lpdis->hDC, 6, yPos, achTemp, cch);

        // Restore the previous colors.
        SetTextColor(lpdis->hDC, clrForeground);
        SetBkColor(lpdis->hDC, clrBackground);

        // If the item has the focus, draw the focus rectangle.
        if (lpdis->itemState & ODS_FOCUS)
            DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
        return true;
    }
    bool onMeasureItem(UINT message, WPARAM wParam, LPARAM lParam) override {
        LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
        lpmis->itemHeight = 20;
        return true;
    }
    void selectItem(const int idx) {
        SendMessageA(hWnd,
            CB_SETCURSEL,
            0,
            0);
    }
    std::string getSelectedText() {
        const int ItemIndex = SendMessage(hWnd, CB_GETCURSEL,
            (WPARAM)0, (LPARAM)0);
        return strings[ItemIndex];
    }
}; 

class ListBox : public Window {
public:
    ListBox(HWND hPar, HINSTANCE hInstance) : Window(hPar) {
        hWnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "ListBox",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NODATA,
            1,
            1,
            1,
            1,
            hPar,
            id,
            hInstance,
            NULL);
    }
    bool onCommand(UINT message, WPARAM wParam, LPARAM lParam) override {
        int x = 1;
        return true;
    }
    void addString(const LBString& str) {
        SendMessageA(hWnd, 
            LB_ADDSTRING,
            0,
            (LPARAM)str.str.c_str());
        strings.push_back(str);
        SendMessageA(hWnd, LB_SETTOPINDEX, strings.size() - 1, 0);

       // UpdateWindow(hWnd);

    }
    bool onDraw(UINT message, WPARAM wParam, LPARAM lParam) override {
        if (strings.empty()){return true;}
        PDRAWITEMSTRUCT pdis = (PDRAWITEMSTRUCT)lParam;
        if (pdis->itemID==-1) { return true; }

        TEXTMETRICA tm;
        size_t cch;
        auto& itemStr = strings[pdis->itemID];
        /*
        // Get the item string from the list box.
        SendMessageA(hWnd, LB_GETTEXT,
            pdis->itemID, (LPARAM)achBuffer);
            */
        // Get the metrics for the current font.
        GetTextMetricsA(pdis->hDC, &tm);
        // Get the character length of the item string.
        cch = itemStr.str.size();
       // hr = StringCchLengthA(itemStr.str.c_str(), 256, &cch);
        // Calculate the vertical position for the item string 
             // so that the string will be vertically centered in the 
             // item rectangle.
        int yPos = (pdis->rcItem.bottom + pdis->rcItem.top -
            tm.tmHeight) / 2;
        auto brush = CreateSolidBrush(itemStr.rgb_bg.toColorRef());
        FillRect(pdis->hDC, (RECT*)&(pdis->rcItem), brush);
        SetBkMode(pdis->hDC, TRANSPARENT);
        SetTextColor(pdis->hDC, itemStr.rgb_fg.toColorRef());
        TextOutA(pdis->hDC, 6, yPos, itemStr.str.c_str(), cch);
        DeleteObject(brush);
        return true;
    }
    std::vector<LBString> strings;
};

class Static : public Window {
public:
    Static(HWND hPar) : Window(hPar), backgroundColor(WHITE) {
        hBrushLabel = NULL;
        hWnd = CreateWindowEx(
            0,
            "STATIC",  // Predefined class; Unicode assumed
            "TX",      // Button text
            SS_CENTER | WS_VISIBLE | WS_CHILD,  // Styles
            10,         // x position
            10,         // y position
            100,        // Button width
            100,        // Button height
            hWndParent, // Parent window
            id,
            (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.
    }
    LRESULT onColorStatic(UINT message, WPARAM wParam, LPARAM lParam) override {
        if (hBrushLabel) {
            DeleteObject(hBrushLabel);
            hBrushLabel = NULL;
        }
        HDC hdc = reinterpret_cast<HDC>(wParam);
        //SetTextColor(hdc, RGB(0,0,0));
        auto oldColor = SetBkColor(hdc, backgroundColor.toColorRef());
        ::GetSysColorBrush(COLOR_WINDOW);
        if (!hBrushLabel) hBrushLabel = CreateSolidBrush(backgroundColor.toColorRef());
        return (LRESULT) hBrushLabel;
    }
    void setText(const std::string& str) {
        SetWindowText(hWnd, (LPCSTR)str.c_str());
    }
    void setBackgroundColor(WinColor c) {
        backgroundColor = c;
        InvalidateRect(hWnd, NULL, TRUE);
    }
    HBRUSH hBrushLabel;
    WinColor backgroundColor;
};

class Button : public Window, public Clickable {
public:
    Button(HWND hPar) : Window(hPar) {
        hWnd = CreateWindow(
            "BUTTON",  // Predefined class; Unicode assumed
            "Button",      // Button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
            10,         // x position
            10,         // y position
            100,        // Button width
            100,        // Button height
            hWndParent,     // Parent window
            id,
            (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.
    }
    bool onCommand(UINT message, WPARAM wParam, LPARAM lParam) override {
        onClick();
        return true;
    }
    void setText(const std::string& str) {
        SetWindowTextA(hWnd, (LPCSTR)str.c_str());
    }
    std::string getText() {
        char c[MAX_PATH] = {0};
        GetWindowTextA(hWnd, c, MAX_PATH);
        return std::string(c);
    }
};

template <typename T, class ...Args>
std::shared_ptr<T> makeWindow(std::shared_ptr<Window> w, Args... args) {
    auto newWindow = std::make_shared<T>(w->hWnd, args...);
    store(newWindow);
    return newWindow;
}

template <typename T, class ...Args>
std::shared_ptr<T> makeWindow(Args... args) {
    auto newWindow = std::make_shared<T>(args...);
    store(newWindow);
    return newWindow;
}

void store(std::shared_ptr<Window> w) {
    windowMap[w->id] = w;
}

std::shared_ptr<Window> getWindowByHandle(HWND hTest) {
    for (auto& w : windowMap) {
        if (w.second->hWnd == hTest) {
            return w.second;
        }
    }
    return nullptr;
}

std::shared_ptr<Window> getWindowById(HMENU h) {
    if (windowMap.find(h) == windowMap.end()) {
        return nullptr;
    }
    else {
        return windowMap[h];
    }
}

LRESULT handleWinMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    bool handled = false;
    std::shared_ptr<Window> w = nullptr;
    if (WM_CTLCOLORSTATIC == message) {
        // lParam is an HWND
        w = getWindowByHandle((HWND)lParam);
    }
    else if (WM_MENUCOMMAND == message) {
        // lParam is an HWND
        w = getWindowByHandle((HWND)lParam);
    }
    else if (WM_DESTROY == message) {
        w = getWindowByHandle(hWnd);
    }
    else {
        // lParam is an ID, aka HMENU
        w = getWindowById((HMENU)LOWORD(wParam));
    }

    if (w) {
        switch (message) {
        case WM_DESTROY:
            handled = w->onDestroy();
            break;
        case WM_CLOSE:
            handled = w->onClose();
            break;
        case WM_MENUCOMMAND:
            handled = w->onMenuCommand(wParam);
            break;
        case WM_COMMAND:
            handled = w->onCommand(message, wParam, lParam);
            break;
        case WM_DRAWITEM:
            return w->onDraw(message, wParam, lParam);
        case WM_MEASUREITEM:
            handled = w->onMeasureItem(message, wParam, lParam);
            break;
        case WM_CTLCOLORSTATIC:
            // return brush
            return w->onColorStatic(message, wParam, lParam);
        default:
            break;
        }
    }

    if (!handled) {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    case WM_CLOSE:
    case WM_DRAWITEM:
    case WM_MENUCOMMAND:
    case WM_COMMAND:
    case WM_MEASUREITEM:
        return handleWinMessage(hWnd, message, wParam, lParam);
    case WM_CTLCOLORSTATIC:
        return handleWinMessage(hWnd, message, wParam, lParam);
    case WM_SIZE:
    {
        auto window = getWindowByHandle(hWnd);
        if (!window) {
            return 0;
        }
        const int width = LOWORD(lParam);
        const int height = HIWORD(lParam);
        return window->onResize(width, height);
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

} // namespace
