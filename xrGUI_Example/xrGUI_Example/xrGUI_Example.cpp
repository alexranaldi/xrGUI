// xrGUI_Example.cpp : Defines the entry point for the application.
//

#include "framework.h"

#include "GUI.hpp"


#define MAX_LOADSTRING 100

struct GUIHandles {
    std::shared_ptr<xrGUI::Window> mainWindow;
    std::shared_ptr<xrGUI::Static> label1;
    std::shared_ptr<xrGUI::Static> label2;
    std::shared_ptr<xrGUI::ListBox> listbox1;
    std::shared_ptr<xrGUI::ListBox> listbox2;
    std::shared_ptr<xrGUI::ComboBox> combobox1;
    std::shared_ptr<xrGUI::EditBox> editbox1;
    std::shared_ptr<xrGUI::Button> button1;
};

// Global Variables:
HINSTANCE hInst;                                // current instance

GUIHandles handles;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClassMainWindow(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    MyRegisterClassMainWindow(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

void onFoobar() {
    // menu callback - put code here
}

void onCloseMainWindow() {
    PostQuitMessage(0);
}

void onChangeCombobox() {
    
}

void onChangeEditBox() {

}

LRESULT onResizeMainWindow(const int w, const int h) {
    const int PADDING = 10;
    const int BUT_H = 40;
    const int BUT_W = 100;
    const int IND_W = 200;
    const int LB_W = 440;
    const int LB_Y = 80;
    const int LBL_H = 20;
    const int LBL_W = LB_W;
    const int LB4_W = 200;
    const int LBL4_W = LB4_W;
    handles.combobox1->setPosition({ 10, 10, 180, 220 });

    const int LB_H = h - (PADDING * 2) - LB_Y;
    const int LB_X = LB_W + (PADDING * 2);
    handles.listbox1->setPosition({ PADDING,LB_Y,LB_W,LB_H });
    handles.listbox2->setPosition({ LB_X * 1,LB_Y,LB_W,LB_H });

    handles.label1->setPosition({ PADDING,LB_Y - 25,LBL_W, LBL_H });
    handles.label2->setPosition({ LB_X * 1,LB_Y - 25,LBL_W, LBL_H });
 
    handles.editbox1->setPosition({ 320, 10, 100, 22 });
    handles.button1->setPosition({ 450, 10, 100, 22 });
    handles.combobox1->setPosition({ 570, 10, 100, 220 });

    return 0; // "If an application processes this message, it should return zero."
}

void createGUI(HINSTANCE hInstance) {
    auto menuBar = xrGUI::makeWindow<xrGUI::Menu>();

    MENUINFO menuInfo;
    memset(&menuInfo, 0, sizeof(menuInfo));
    menuInfo.cbSize = sizeof(MENUINFO);
    menuInfo.fMask = MIM_STYLE;
    menuInfo.dwStyle = MNS_NOTIFYBYPOS;
    SetMenuInfo((HMENU)menuBar->hWnd, &menuInfo);

    auto menuView = xrGUI::makeWindow<xrGUI::Menu>();

    menuBar->addSubMenu(menuView, "File");
    auto waterfallId = menuView->addTextItem("Foobar");

    menuView->setClickCallback(waterfallId, &onFoobar);

    handles.mainWindow = xrGUI::makeWindow< xrGUI::MainWindow >(hInstance, "MainWindow", "My Window", (HMENU)menuBar->hWnd);
    handles.mainWindow->setResizeCallback(&onResizeMainWindow);
    handles.mainWindow->setDestroyCallback(&onCloseMainWindow);

    handles.label1 = xrGUI::makeWindow<xrGUI::Static>(handles.mainWindow->hWnd);
    handles.label1->setText("Listbox 1");
    handles.label1->setFont("Arial", 11);

    handles.label2 = xrGUI::makeWindow<xrGUI::Static>(handles.mainWindow->hWnd);
    handles.label2->setText("Listbox 2");
    handles.label2->setFont("Arial", 11);

    handles.listbox1 = xrGUI::makeWindow<xrGUI::ListBox>(handles.mainWindow->hWnd, hInstance);
    handles.listbox1->setFont("Consolas", 12);

    handles.listbox2 = xrGUI::makeWindow<xrGUI::ListBox>(handles.mainWindow->hWnd, hInstance);
    handles.listbox2->setFont("Consolas", 11);


    handles.combobox1 = xrGUI::makeWindow<xrGUI::ComboBox>(handles.mainWindow->hWnd, hInstance);
    handles.combobox1->addString("Apple");
    handles.combobox1->addString("Orange");
    handles.combobox1->addString("Plum");
    handles.combobox1->selectItem(0);
    handles.combobox1->setClickCallback(std::bind(&onChangeCombobox));

    handles.editbox1 = xrGUI::makeWindow<xrGUI::EditBox>(handles.mainWindow->hWnd, hInstance);
    handles.editbox1->setText("Edit Box");
    handles.editbox1->setFont("Consolas", 11);
    handles.editbox1->setClickCallback(&onChangeEditBox);

    handles.button1 = xrGUI::makeWindow<xrGUI::Button>(handles.mainWindow->hWnd);
    handles.button1->setText("A B C");
    handles.button1->setClickCallback(&onChangeEditBox);

}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    createGUI(hInstance);

    handles.mainWindow->show(SW_SHOWDEFAULT);

    return TRUE;
}

ATOM MyRegisterClassMainWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    HBRUSH hBrush = CreateSolidBrush(xrGUI::GREY_BLUE.toColorRef());

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = 0;
    wcex.lpfnWndProc = xrGUI::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = hBrush;//(HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "MainWindow";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

    return RegisterClassExA(&wcex);
}
