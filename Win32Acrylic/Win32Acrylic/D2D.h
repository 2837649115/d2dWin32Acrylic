#pragma once
#include <Windows.h>
#include <ShellScalingApi.h>
#include <wrl.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <array>
#include <random>
#include <dwrite_2.h>
#include <wincodec.h>

#include"Debug.h"

#pragma comment(lib, "shcore")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "dwrite")

using namespace Microsoft::WRL;
using namespace D2D1;



struct ComException
{
    HRESULT result;
    ComException(HRESULT const value);
};
void HR(HRESULT const result);

template <typename T>
static float PhysicalToLogical(T const pixel,
    float const dpi)
{
    return pixel * 96.0f / dpi;
}

template <typename T>
static float LogicalToPhysical(T const pixel,
    float const dpi)
{
    return pixel * dpi / 96.0f;
}

struct RECTF
{
    FLOAT top;
    FLOAT left;
    FLOAT right;
    FLOAT bottom;
};

void CreateDevice3D(ComPtr<ID3D11Device>& m_device3D);

bool IsDeviceCreated(const ComPtr<ID3D11Device>& m_device3D);

void CreateDevice2D(ComPtr<ID3D11Device>const& m_device3D, ComPtr<ID2D1Device>& device2D);

void CreateSurface(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionSurface>& surface,
    UINT width, UINT height);

void CreateTextFormat(ComPtr<IDWriteTextFormat>& textFormat, const WCHAR* fontName, FLOAT fontSize);

void ReleaseDeviceResources(ComPtr<ID3D11Device>& m_device3D);

void CreateVisual(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionVisual2>& visual);

void D2DDrawText(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionVisual2>const& visual,
    ComPtr<IDWriteTextFormat>const& textFormat,
    const WCHAR* string,
    const FLOAT left, const FLOAT top,
    const FLOAT right, const FLOAT bottom);

void CreateDeviceResources(HWND hWnd,
    ComPtr<ID3D11Device>& m_device3D,
    ComPtr<IDCompositionDesktopDevice>& m_device,
    ComPtr<IDCompositionTarget>& m_target,
    ComPtr<IDCompositionVisual2>& rootVisual);

void WindowDraw(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionVisual2>const& rootVisual);

class  RoundButton
{
protected:
    RECTF rect;
    float defaultRadius;
    float radius;
    float diameter;

public:
    ComPtr<IDCompositionVisual2> ButtonVisual;
    ComPtr<IDCompositionVisual2> ButtonDownVisual;
	
    void CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device,
        float left, float top,
        float diameter);
    
    
    bool IsMouseIn(LONG X, LONG Y);

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device,
        float left, float top);

    void Move(LONG left, LONG top);

    virtual void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device) = 0;
    
    virtual void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device) = 0;

};

class  CloseButton : public RoundButton
{
public:
    void CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device);

    bool IsMouseIn(LONG X, LONG Y);

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    LONG GetLeft();

    LONG GetTop();
};

class  MinButton : public RoundButton
{
public:

    void CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device);

    bool IsMouseIn(LONG X, LONG Y);

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    LONG GetRight();

    LONG GetBottom();
};

class RoundedRectangleButton
{
private:
    RECTF defaultRect, rect;
    float defaultWidth, defaultHeight, width, height;
    float defaultDiameter, diameter;

    ComPtr<IDWriteTextFormat> textFormat;
public:
    ComPtr<IDCompositionVisual2> ButtonVisual;
    ComPtr<IDCompositionVisual2> ButtonDownVisual;
    ComPtr<IDCompositionVisual2> textVisual;

    void CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device,
        FLOAT left, FLOAT top,
        FLOAT width, FLOAT height,
        FLOAT diameter);
    
    bool IsMouseIn(LONG X, LONG Y);

    void ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device);

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device);
};

class Label
{
private:
    ComPtr<IDWriteTextFormat> textFormat;
    const WCHAR* string;
    FLOAT left, top, right, bottom;
public:
    ComPtr<IDCompositionVisual2> LabelVisual;

    void CreateLabel(ComPtr<IDCompositionDesktopDevice>const& m_device,
        const WCHAR* fontName,
        FLOAT fontsize,
        const WCHAR* string,
        FLOAT left, FLOAT top, FLOAT right, FLOAT bottom);
    void ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device);
};