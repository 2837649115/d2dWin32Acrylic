// Win32Acrylic.cpp : 定义应用程序的入口点。

#include "main.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例

WCHAR szAppTitle[MAX_LOADSTRING];               // 窗口标题
WCHAR szShadowWndClass[MAX_LOADSTRING];            //阴影窗口类名
WCHAR szAcrylicWndTitle[MAX_LOADSTRING];            //亚克力窗口标题
WCHAR szAcrylicWndClass[MAX_LOADSTRING];             // 亚克力窗口类名
WCHAR szMainWndTitle[MAX_LOADSTRING];               // 主窗口标题
WCHAR szMainWndClass[MAX_LOADSTRING];               // 主窗口类名

HWND  hShadowWnd, hAcrylicWnd, hMainWnd;            //窗口句柄

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

LRESULT CALLBACK    ShadowWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    AcrylicWndProc(HWND, UINT, WPARAM, LPARAM);


LRESULT CALLBACK    MainWndProc(HWND, UINT, WPARAM, LPARAM);

ComPtr<ID3D11Device> m_device3D;
ComPtr<IDCompositionDesktopDevice> m_device;
ComPtr<IDCompositionTarget> m_target;
ComPtr<IDCompositionVisual2> rootVisual;

ComPtr<IDWriteTextFormat> m_textFormat;

Label label;
CloseButton closeButton;
MinButton minButton;
RoundedRectangleButton button;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szAppTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_DAWWND_CLASS, szShadowWndClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_ACLWND_CLASS, szAcrylicWndClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_DAWWND_TITLE, szAcrylicWndTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_EVTWND_TITLE, szMainWndTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_EVTWND_CLASS, szMainWndClass, MAX_LOADSTRING);

    // TODO: 在此处放置代码。


    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }


    
    // 主消息循环:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ShadowWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32ACRYLIC));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));    //(HBRUSH) GetStockObject(GRAY_BRUSH);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szShadowWndClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    wcex.lpfnWndProc = AcrylicWndProc;
    wcex.lpszClassName = szAcrylicWndClass;

    RegisterClassExW(&wcex);

    wcex.lpfnWndProc = MainWndProc;
    wcex.lpszClassName = szMainWndClass;

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
void GetDPI(float* m_dpiX, float* m_dpiY)
{
    //设置DPI感知模式
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    HMONITOR const monitor = MonitorFromWindow(NULL,    //hWnd
    MONITOR_DEFAULTTONEAREST);

    unsigned dpiX = 0;
    unsigned dpiY = 0;

    HR(GetDpiForMonitor(monitor,
        MDT_EFFECTIVE_DPI,
        &dpiX,
        &dpiY));

    *m_dpiX = dpiX;
    *m_dpiY = dpiY;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    GetDPI(&m_dpiX, &m_dpiY);
    WindowWidth = static_cast<unsigned>(LogicalToPhysical(defaultWindowWidth, m_dpiX));
    WindowHeight = static_cast<unsigned>(LogicalToPhysical(defaultWindowHeight, m_dpiY));

    //获取屏幕尺寸
    //获取窗体尺寸
    RECT rect;
    rect.right = GetSystemMetrics(SM_CXSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYSCREEN);

    //窗口居中顶点坐标
    rect.left = (rect.right - WindowWidth) / 2;
    rect.top = (rect.bottom - WindowHeight) / 2;

    //创建窗口
    hShadowWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP, szShadowWndClass, szAppTitle, WS_POPUP,
        rect.left, rect.top, WindowWidth, WindowHeight, nullptr, nullptr, hInstance, nullptr);
    if (!hShadowWnd)
        return FALSE;
    //窗口圆角
    SetWindowRgn(hShadowWnd, CreateRoundRectRgn(0, 0, WindowWidth, WindowHeight, 20, 20), FALSE);

    hAcrylicWnd = CreateWindowEx(WS_EX_LAYERED,szAcrylicWndClass, szAcrylicWndTitle, WS_POPUP,
        rect.left, rect.top, WindowWidth, WindowHeight, hShadowWnd, nullptr, hInstance, nullptr);
    if (!hAcrylicWnd)
        return FALSE;
    /*------------------------------------*/
    SetWindowAcrylic(hAcrylicWnd, ACCENT_ENABLE_ACRYLICBLURBEHIND, WindowWidth, WindowHeight, 20);
    /*------------------------------------*/
    hMainWnd = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, szMainWndClass, szMainWndTitle, WS_POPUP,
       rect.left, rect.top, WindowWidth, WindowHeight, hAcrylicWnd, nullptr, hInstance, nullptr);
    if (!hMainWnd)
        return FALSE;
    //窗口圆角
    SetWindowRgn(hMainWnd, CreateRoundRectRgn(0, 0, WindowWidth, WindowHeight, 20, 20), FALSE);

    //显示窗口
    ShowWindow(hAcrylicWnd, nCmdShow);
    ShowWindow(hShadowWnd, nCmdShow);
    ShowWindow(hMainWnd, nCmdShow);
  
    //窗口更新
    UpdateWindow(hAcrylicWnd);
    UpdateWindow(hShadowWnd);
    UpdateWindow(hMainWnd);

    return TRUE;
}


//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
void WindowRectChange(int left, int top,
    int cx, int cy,
    UINT uFlags = NULL)  //SWP_NOMOVE\SWP_NOSIZE
{
    if (uFlags != SWP_NOSIZE)
    {
        SetWindowRgn(hShadowWnd, CreateRoundRectRgn(0, 0, cx, cy, 20, 20), FALSE);
        SetWindowRgn(hAcrylicWnd, CreateRoundRectRgn(0, 0, cx, cy, 20, 20), FALSE);
        SetWindowRgn(hMainWnd, CreateRoundRectRgn(0, 0, cx, cy, 20, 20), FALSE);
    }
    HDWP hdwp = BeginDeferWindowPos(3);

    hdwp = DeferWindowPos(hdwp, hShadowWnd, NULL, left, top,
        cx, cy, SWP_NOACTIVATE | SWP_NOZORDER |
        uFlags);
    hdwp = DeferWindowPos(hdwp, hAcrylicWnd, NULL, left, top,
        cx, cy, SWP_NOACTIVATE | SWP_NOZORDER |
        uFlags);
    hdwp = DeferWindowPos(hdwp, hMainWnd, NULL, left, top,
        cx, cy, SWP_NOACTIVATE | SWP_NOZORDER |
        uFlags);
    
    EndDeferWindowPos(hdwp);
}
void DpiChangedHandler(HWND hWnd, WPARAM const wparam, LPARAM const lparam)
{
    m_dpiX = LOWORD(wparam);
    m_dpiY = HIWORD(wparam);

    WindowWidth = LogicalToPhysical(defaultWindowWidth, m_dpiX);
    WindowHeight = LogicalToPhysical(defaultWindowHeight, m_dpiY);

    RECT const* suggested =
        reinterpret_cast<RECT const*>(lparam);

    WindowRectChange(suggested->left,
        suggested->top,
        suggested->right - suggested->left,
        suggested->bottom - suggested->top );
}

void CreateHandler(HWND hWnd)
{
    
}

void PaintHandler(HWND hWnd)
{
    try
    {
        if (IsDeviceCreated(m_device3D))
        {
            HR(m_device3D->GetDeviceRemovedReason());
        }
        else
        {
            CreateDeviceResources(hWnd, m_device3D, m_device, m_target, &rootVisual);
        }

        VERIFY(ValidateRect(hWnd, nullptr));
    }
    catch (ComException const& e)
    {
        TRACE(L"PaintHandler failed 0x%X\n", e.result);

        ReleaseDeviceResources(&m_device3D);
    }
}

BOOL lButtonDown = FALSE;
POINT MousePint;

BOOL Acrylic = TRUE;
BOOL MouseIn = FALSE;
unsigned moveID = 0, downID = 0;    //响应事件的按钮ID
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    
    RECT rect;
    LONG X, Y;
    switch (message)
    {
    case WM_CREATE:
        CreateHandler(hWnd);

        CreateDeviceResources(hWnd, m_device3D, m_device, m_target, &rootVisual);
        label.CreateLabel(m_device, L"Tahoma", 40.0F, L"Direct2D Sample", 0.0F, 0.0F, defaultWindowWidth, defaultWindowHeight);
        closeButton.CreateButton(m_device);
        minButton.CreateButton(m_device);
        button.CreateButton(m_device, 155.0F, 220.0F, 90.0F, 40.0F, 8.0F);

        WindowDraw(m_device, rootVisual);

        HR(rootVisual->AddVisual(label.LabelVisual.Get(), false, nullptr));
        HR(rootVisual->AddVisual(closeButton.ButtonVisual.Get(), false, nullptr));
        HR(rootVisual->AddVisual(minButton.ButtonVisual.Get(), false, nullptr));
        HR(rootVisual->AddVisual(button.ButtonVisual.Get(), false, nullptr));
        HR(m_device->Commit());

        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        PaintHandler(hWnd);

        EndPaint(hWnd, &ps);
        break;
    case WM_LBUTTONDOWN:
        

        MousePint.x = LOWORD(lParam);
        MousePint.y = HIWORD(lParam);

        

        if (button.IsMouseIn(MousePint.x, MousePint.y))
        {
            button.ButtonDown(m_device);
            downID = 3;
            MessageBox(hWnd, L"this is a button!", L"提示", MB_OK);
            SendMessage(hWnd, WM_LBUTTONUP, NULL, NULL);
        }
        else if (closeButton.IsMouseIn(MousePint.x, MousePint.y))
        {
            SendMessage(hWnd, WM_CLOSE, NULL, NULL);
        }
        else if (minButton.IsMouseIn(MousePint.x, MousePint.y))
        {
            SendMessage(hShadowWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);

            closeButton.ButtonUp(m_device);
            minButton.ButtonUp(m_device);

            HR(m_device->Commit());

            moveID = 0;
        }
        else
        {
            lButtonDown = TRUE;
        }

        break;
    case WM_LBUTTONUP:

        switch (downID)
        {
        case 3:
            button.ButtonUp(m_device);
            downID = 0;
            break;
        }

        if (lButtonDown)
        {
            lButtonDown = FALSE;

            if (Acrylic == FALSE)
            {
                OnWindowAcrylic(hAcrylicWnd, ACCENT_ENABLE_ACRYLICBLURBEHIND);
                Acrylic = TRUE;
            }
        }
        
        break;
    case WM_MOUSEMOVE:
        X = LOWORD(lParam);
        Y = HIWORD(lParam);

        if (X > closeButton.GetLeft() && X < minButton.GetRight() &&
            Y > closeButton.GetTop() && Y < minButton.GetBottom())
        {   
            switch (moveID)
            {
            case 0:
                closeButton.ButtonDown(m_device);
                minButton.ButtonDown(m_device);

                HR(m_device->Commit());

                moveID = 1;
                break;
            }
            
        }
        else
        {
            switch (moveID)
            {
            case 1:
                closeButton.ButtonUp(m_device);
                minButton.ButtonUp(m_device);

                HR(m_device->Commit());

                moveID = 0;
                break;
            }

            if (lButtonDown)
            {
                if (!MouseIn)
                {
                    MouseIn = TRUE;

                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(tme);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hWnd;
                    tme.dwHoverTime = 0;
                    TrackMouseEvent(&tme);
                }
                
                if (Acrylic)
                {
                    OnWindowAcrylic(hAcrylicWnd, ACCENT_ENABLE_BLURBEHIND);
                    Acrylic = FALSE;
                }

                GetWindowRect(hWnd, &rect);
                rect.left += X - MousePint.x;
                rect.top += Y - MousePint.y;

				WindowRectChange(rect.left, rect.top,
					0, 0,
					SWP_NOSIZE);
            }
        }
        
        break;
    case WM_MOUSELEAVE:
        MouseIn = FALSE;
        if (lButtonDown)
        {
            lButtonDown = FALSE;
            OnWindowAcrylic(hAcrylicWnd, ACCENT_ENABLE_ACRYLICBLURBEHIND);
            Acrylic = TRUE;
        }
        break;
	case WM_ACTIVATE:
		break;
    case WM_RBUTTONDOWN:
        break;
    case WM_DESTROY:
        ReleaseDeviceResources(&m_device3D);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



LRESULT CALLBACK AcrylicWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK ShadowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DPICHANGED:
        DpiChangedHandler(hWnd, wParam, lParam);

        WindowDraw(m_device, rootVisual);

        label.ChangeDPI(m_device);
        closeButton.ChangeDPI(m_device, 10.0F, 10.0F);
        minButton.ChangeDPI(m_device, 40.0F, 10.0F);
        button.ChangeDPI(m_device);
        HR(m_device->Commit());
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}