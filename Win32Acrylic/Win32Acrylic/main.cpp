// Win32Acrylic.cpp : 定义应用程序的入口点。
#include "main.h"

extern NOTIFYICONDATA nid;		//托盘属性
extern HMENU hMenu;			//托盘菜单

//GdiPlus
extern GdiplusStartupInput gdiplusStartupInput;
extern ULONG_PTR pGdiToken;

unsigned WindowWidth = 400.0F;
unsigned WindowHeight = 300.0F;
unsigned WindowRoundD = 20.0F;
unsigned shadowWidth = 20;
FLOAT m_dpi = 96.0F, beforeDpi = 96.0F;

// 全局变量:
HINSTANCE hInst;                                // 当前实例

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
    wcex.lpszClassName = DAWWND_CLASS;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    wcex.lpfnWndProc = AcrylicWndProc;
    wcex.lpszClassName = ACLWND_CLASS;

    RegisterClassExW(&wcex);

    wcex.lpfnWndProc = MainWndProc;
    wcex.lpszClassName = MANWND_CLASS;

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
void GetDPI(float* m_dpi)
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

    *m_dpi = dpiX;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    GetDPI(&m_dpi);
    WindowWidth = LogicalToPhysical(WindowWidth, m_dpi, beforeDpi);
    WindowHeight = LogicalToPhysical(WindowHeight, m_dpi, beforeDpi);
    WindowRoundD = LogicalToPhysical(WindowRoundD, m_dpi, beforeDpi);
    shadowWidth = LogicalToPhysical(shadowWidth, m_dpi, beforeDpi);

    //获取屏幕尺寸
    //获取窗体尺寸
    RECT rect;
    rect.right = GetSystemMetrics(SM_CXSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYSCREEN);

    //窗口居中顶点坐标
    rect.left = (rect.right - WindowWidth) / 2;
    rect.top = (rect.bottom - WindowHeight) / 2;
    //创建窗口
    VERIFY(CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        DAWWND_CLASS, DAWWND_TITLE,
        WS_POPUP,
        rect.left - shadowWidth, rect.top - shadowWidth,
        WindowWidth + shadowWidth * 2, WindowHeight + shadowWidth * 2,
        nullptr,
        nullptr,
        hInstance,
        nullptr));

    VERIFY(CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT,
        ACLWND_CLASS, ACLWND_TITLE,
        WS_POPUP,
        rect.left + 1, rect.top + 1,
        WindowWidth - 2, WindowHeight - 2,
        hShadowWnd,
        nullptr,
        hInstance,
        nullptr));
    
    /*------------------------------------*/
    SetWindowAcrylic(hAcrylicWnd,
        ACCENT_ENABLE_ACRYLICBLURBEHIND,
        WindowWidth - 2, WindowHeight - 2,
        WindowRoundD);
    /*------------------------------------*/
    VERIFY(CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP,
        MANWND_CLASS, APP_TITLE,
        WS_POPUP,
        rect.left, rect.top,
        WindowWidth, WindowHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr));
    //窗口圆角
    SetWindowRgn(hMainWnd,
        CreateRoundRectRgn(0, 0, WindowWidth, WindowHeight, WindowRoundD, WindowRoundD),
        FALSE);

    //显示窗口
    ShowWindow(hShadowWnd, nCmdShow);
    ShowWindow(hAcrylicWnd, nCmdShow);
    ShowWindow(hMainWnd, nCmdShow);
  
    //窗口更新
    UpdateWindow(hShadowWnd);
    UpdateWindow(hAcrylicWnd);
    UpdateWindow(hMainWnd);

    InitTray(hInstance, hMainWnd);			//实例化托盘
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
void SetWindowRect(int left, int top,
    int cx, int cy,
    UINT uFlags = NULL)
{
    HDWP hdwp = BeginDeferWindowPos(3);
    switch (uFlags)
    {
    case NULL:
        SetWindowRgn(hAcrylicWnd, CreateRoundRectRgn(0, 0, cx - 2, cy - 2, WindowRoundD, WindowRoundD), FALSE);
        SetWindowRgn(hMainWnd, CreateRoundRectRgn(0, 0, cx, cy, WindowRoundD, WindowRoundD), FALSE);

        hdwp = DeferWindowPos(hdwp, hShadowWnd, NULL, left - shadowWidth, top -shadowWidth,
            cx + shadowWidth * 2, cy + shadowWidth * 2,
            SWP_NOACTIVATE | SWP_NOZORDER);
        hdwp = DeferWindowPos(hdwp, hAcrylicWnd, NULL, left + 1, top + 1,
            cx - 2, cy - 2,
            SWP_NOACTIVATE | SWP_NOZORDER);
        hdwp = DeferWindowPos(hdwp, hMainWnd, NULL, left, top,
            cx, cy,
            SWP_NOACTIVATE | SWP_NOZORDER);
    
        EndDeferWindowPos(hdwp);
        break;
    case SWP_NOMOVE:
        SetWindowRgn(hAcrylicWnd, CreateRoundRectRgn(0, 0, cx - 2, cy - 2, WindowRoundD, WindowRoundD), FALSE);
        SetWindowRgn(hMainWnd, CreateRoundRectRgn(0, 0, cx, cy, WindowRoundD, WindowRoundD), FALSE);

        hdwp = DeferWindowPos(hdwp, hShadowWnd, NULL, 0, 0,
            cx + shadowWidth * 2, cy + shadowWidth * 2, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        hdwp = DeferWindowPos(hdwp, hAcrylicWnd, NULL, 0, 0,
            cx - 2, cy - 2, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        hdwp = DeferWindowPos(hdwp, hMainWnd, NULL, 0, 0,
            cx, cy,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

        EndDeferWindowPos(hdwp);
        break;
    case SWP_NOSIZE:
        hdwp = DeferWindowPos(hdwp, hShadowWnd, NULL, left - shadowWidth, top - shadowWidth,
            0, 0,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
        hdwp = DeferWindowPos(hdwp, hAcrylicWnd, NULL, left + 1, top + 1,
            0, 0,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
        hdwp = DeferWindowPos(hdwp, hMainWnd, NULL, left, top,
            0, 0,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

        EndDeferWindowPos(hdwp);
        break;
    }
}

void ChangeWindowAfter()
{
    HDWP hdwp = BeginDeferWindowPos(2);

    hdwp = DeferWindowPos(hdwp, hMainWnd, NULL, 0, 0,
        0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    hdwp = DeferWindowPos(hdwp, hShadowWnd, hMainWnd, 0, 0,
        0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

    EndDeferWindowPos(hdwp);
}

void DpiChangedHandler(HWND hWnd, WPARAM const wparam, LPARAM const lparam)
{
    beforeDpi = m_dpi;
    
    m_dpi = LOWORD(wparam);
    //m_dpiY = HIWORD(wparam);
    RECT const* suggested =
        reinterpret_cast<RECT const*>(lparam);

    WindowWidth = suggested->right - suggested->left;
    WindowHeight = suggested->bottom - suggested->top;
    WindowRoundD = LogicalToPhysical(WindowRoundD, m_dpi, beforeDpi);
    shadowWidth = LogicalToPhysical(shadowWidth, m_dpi, beforeDpi);

    SetWindowRect(suggested->left,
        suggested->top,
        WindowWidth,
        WindowHeight);
}


extern BOOL lButtonDown;
extern POINT MousePoint;

extern BOOL bAcrylic;
extern BOOL bMouseIn;
extern BOOL bHide;
extern unsigned moveID, downID;
RECT wndRect;
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    
    
    LONG X, Y;
    POINT pPos;
    switch (message)
    {
    case WM_ACTIVATEAPP:
        ChangeWindowAfter();
        break;
    case WM_ACTIVATE:
        break;
    case WM_CREATE:
        hMainWnd = hWnd;

        CreateHandler(hWnd);
        ChangeWindowAfter();

        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        PaintHandler(hWnd);

        EndPaint(hWnd, &ps);
        break;
    case WM_LBUTTONDOWN:

        if (!bMouseIn)
        {
            bMouseIn = TRUE;

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            tme.dwHoverTime = 0;
            TrackMouseEvent(&tme);
        }

        X = LOWORD(lParam);
        Y = HIWORD(lParam);

        if (button.IsMouseIn(X, Y))
        {
            button.ButtonDown(m_device);
            downID = 3;
            //MessageBox(hWnd, L"this is a button!", L"提示", MB_OK);
            //SendMessage(hWnd, WM_LBUTTONUP, NULL, NULL);
        }
        else if (closeButton.IsMouseIn(X, Y))
        {
            //SendMessage(hWnd, WM_CLOSE, NULL, NULL);
            ShowTrayMsg();
            
            ShowWindow(hWnd, SW_HIDE);
            ShowWindow(hAcrylicWnd, SW_HIDE);
            ShowWindow(hShadowWnd, SW_HIDE);

            closeButton.ButtonUp(m_device);
            minButton.ButtonUp(m_device);

            HR(m_device->Commit());

            bHide = TRUE;
            moveID = 0;
        }
        else if (minButton.IsMouseIn(X, Y))
        {
            SendMessage(hMainWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
            SendMessage(hShadowWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);

            closeButton.ButtonUp(m_device);
            minButton.ButtonUp(m_device);

            HR(m_device->Commit());

            moveID = 0;
        }
        else
        {
            GetWindowRect(hWnd, &wndRect);
            GetCursorPos(&MousePoint);
            lButtonDown = TRUE;
            //设置鼠标捕获
            SetCapture(hWnd);
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
            ReleaseCapture();

            lButtonDown = FALSE;

            if (bAcrylic == FALSE)
            {
                OnWindowAcrylic(hAcrylicWnd, ACCENT_ENABLE_ACRYLICBLURBEHIND);
                bAcrylic = TRUE;
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
                if (bAcrylic)
                {
                    OnWindowAcrylic(hAcrylicWnd, ACCENT_ENABLE_BLURBEHIND);
                    bAcrylic = FALSE;
                }

                GetCursorPos(&pPos);
                wndRect.right = pPos.x - MousePoint.x + wndRect.left;
                wndRect.bottom = pPos.y - MousePoint.y + wndRect.top;

				SetWindowRect(wndRect.right, wndRect.bottom,
					0, 0,
					SWP_NOSIZE);
            }
        }
        
        break;
    case WM_MOUSELEAVE:
        bMouseIn = FALSE;
        SendMessage(hWnd, WM_LBUTTONUP, NULL, NULL);
        break;
    case WM_RBUTTONDOWN:
        break;
    case WM_TRAY:
        switch (lParam)
        {
        case WM_RBUTTONDOWN:
        {
            //获取鼠标坐标
            POINT pt; GetCursorPos(&pt);

            //解决在菜单外单击左键菜单不消失的问题
            SetForegroundWindow(hWnd);

            //使菜单某项变灰
            //EnableMenuItem(hMenu, ID_SHOW, MF_GRAYED);	

            //显示并获取选中的菜单
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd,
                NULL);
            if (cmd == ID_SHOW)
                MessageBox(hWnd, APP_TIP, APP_NAME, MB_OK);
            if (cmd == ID_EXIT)
                PostMessage(hWnd, WM_DESTROY, NULL, NULL);
        }
        break;
        case WM_LBUTTONDOWN:
            if (bHide)
            {
                ShowWindow(hShadowWnd, SW_SHOW);
                ShowWindow(hAcrylicWnd, SW_SHOW);
                ShowWindow(hMainWnd, SW_SHOW);
                bHide = FALSE;
            }
            else
            {
                SendMessage(hMainWnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
            }
            //MessageBox(hWnd, APP_TIP, APP_NAME, MB_OK);
            break;
        case WM_LBUTTONDBLCLK:
            break;
        }
        break;
    case WM_DPICHANGED:
        DpiChangedHandler(hWnd, wParam, lParam);

        gdiPlusDraw(hShadowWnd);

        WindowDraw(m_device, rootVisual);

        label.ChangeDPI(m_device);
        closeButton.ChangeDPI(m_device, 10.0F, 10.0F);
        minButton.ChangeDPI(m_device, 40.0F, 10.0F);
        button.ChangeDPI(m_device);
        HR(m_device->Commit());
        break;
    case WM_DESTROY:
        ReleaseDeviceResources(m_device3D);

        //窗口销毁时删除托盘
        Shell_NotifyIcon(NIM_DELETE, &nid);

        PostQuitMessage(0);
        break;
    case WM_SYSCOMMAND:
        if (wParam == SC_RESTORE)
            SendMessage(hShadowWnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



LRESULT CALLBACK AcrylicWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hAcrylicWnd = hWnd;
        break;
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
    case WM_CREATE:
        hShadowWnd = hWnd;
        //初始化GDI+
        GdiplusStartup(&pGdiToken, &gdiplusStartupInput, NULL);  
        gdiPlusDraw(hWnd);
        break;
    case WM_DESTROY:
        GdiplusShutdown(pGdiToken);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}