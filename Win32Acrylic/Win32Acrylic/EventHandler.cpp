#include "EventHandler.h"

extern unsigned WindowWidth;
extern unsigned WindowHeight;

extern FLOAT m_dpi, beforeDpi;

extern ComPtr<ID3D11Device> m_device3D;
extern ComPtr<IDCompositionDesktopDevice> m_device;
extern ComPtr<IDCompositionTarget> m_target;
extern ComPtr<IDCompositionVisual2> rootVisual;

extern HWND  hShadowWnd, hAcrylicWnd, hMainWnd;

extern ComPtr<IDWriteTextFormat> m_textFormat;


extern Label label;
extern CloseButton closeButton;
extern MinButton minButton;
extern RoundedRectangleButton button;


BOOL lButtonDown = FALSE;
POINT MousePoint;

BOOL bAcrylic = TRUE;
BOOL bMouseIn = FALSE;
BOOL bHide = FALSE;
unsigned moveID = 0, downID = 0;    //响应事件的按钮ID

void CreateHandler(HWND hWnd)
{
    CreateDeviceResources(hWnd, m_device3D, m_device, m_target, rootVisual);
    label.CreateLabel(m_device, L"Tahoma", 40.0F, L"Direct2D Sample", 0.0F, 0.0F, 400, 300);
    closeButton.CreateButton(m_device);
    minButton.CreateButton(m_device);
    button.CreateButton(m_device, 155.0F, 220.0F, 90.0F, 40.0F, 8.0F);

    WindowDraw(m_device, rootVisual);

    HR(rootVisual->AddVisual(label.LabelVisual.Get(), false, nullptr));
    HR(rootVisual->AddVisual(closeButton.ButtonVisual.Get(), false, nullptr));
    HR(rootVisual->AddVisual(minButton.ButtonVisual.Get(), false, nullptr));
    HR(rootVisual->AddVisual(button.ButtonVisual.Get(), false, nullptr));
    HR(m_device->Commit());
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
            CreateDeviceResources(hWnd, m_device3D, m_device, m_target, rootVisual);
        }

        VERIFY(ValidateRect(hWnd, nullptr));
    }
    catch (ComException const& e)
    {
        TRACE(L"PaintHandler failed 0x%X\n", e.result);

        ReleaseDeviceResources(m_device3D);
    }
}