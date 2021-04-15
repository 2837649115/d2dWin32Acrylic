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

static float WindowWidth = 400.0f;
static float WindowHeight = 300.0f;

static float m_dpiX = 96.0f;
static float m_dpiY = 96.0f;

struct ComException
{
    HRESULT result;
    ComException(HRESULT const value) :
        result(value)
    {}
};
void HR(HRESULT const result)
{
    if (S_OK != result)
    {
        throw ComException(result);
    }
}

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
    float top;
    float left;
    float right;
    float bottom;
};

void CreateDevice3D(ComPtr<ID3D11Device>& m_device3D)
{
    //ASSERT(!IsDeviceCreated(m_device3D));


    unsigned flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
        D3D11_CREATE_DEVICE_SINGLETHREADED;


    #ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    HR(D3D11CreateDevice(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        nullptr, 0,
        D3D11_SDK_VERSION,
        m_device3D.GetAddressOf(),
        nullptr,
        nullptr));

}

bool IsDeviceCreated(const ComPtr<ID3D11Device>& m_device3D)
{
    return m_device3D;
}

ComPtr<ID2D1Device> CreateDevice2D(ComPtr<ID3D11Device>& m_device3D)
{
    ComPtr<IDXGIDevice3> deviceX;
    HR(m_device3D.As(&deviceX));

    D2D1_CREATION_PROPERTIES properties = {};
    #ifdef _DEBUG
    properties.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    #endif


    ComPtr<ID2D1Device> device2D;

    HR(D2D1CreateDevice(deviceX.Get(),
        properties,
        device2D.GetAddressOf()));

    return device2D;
}

template <typename T>
ComPtr<IDCompositionSurface> CreateSurface(ComPtr<IDCompositionDesktopDevice>const& m_device,
    T const width, T const height)
{
    ComPtr<IDCompositionSurface> surface;

    HR(m_device->CreateSurface(static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_ALPHA_MODE_PREMULTIPLIED,
        surface.GetAddressOf()));

    return surface;
}

void CreateTextFormat(ComPtr<IDWriteTextFormat>& m_textFormat)
{
    ComPtr<IDWriteFactory2> factory;

    HR(DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(factory),
        reinterpret_cast<IUnknown**>(factory.GetAddressOf())));

    HR(factory->CreateTextFormat(L"Tahoma",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        40.0f,
        L"",
        m_textFormat.GetAddressOf()));

    HR(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    HR(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
}

void ReleaseDeviceResources(ComPtr<ID3D11Device>& m_device3D)
{
    m_device3D.Reset();
}

ComPtr<IDCompositionVisual2> CreateVisual(ComPtr<IDCompositionDesktopDevice>const& m_device)
{
    ComPtr<IDCompositionVisual2> visual;

    HR(m_device->CreateVisual(visual.GetAddressOf()));

    return visual;
}

void DrawCardFront(ComPtr<IDWriteTextFormat>const& m_textFormat,
    ComPtr<IDCompositionSurface> const& surface)
{
    ComPtr<ID2D1DeviceContext> dc;
    POINT offset = {};

    HR(surface->BeginDraw(nullptr,
        __uuidof(dc),
        reinterpret_cast<void**>(dc.GetAddressOf()),
        &offset));

    dc->SetDpi(m_dpiX,
        m_dpiY);
    dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
        PhysicalToLogical(offset.y, m_dpiY)));
    // Draw something
    dc->Clear();

    ComPtr<ID2D1SolidColorBrush> brush;
    D2D1_COLOR_F const color = ColorF(0.0f, // red
        0.0f, // green
        0.0f, // blue
        1.0f); // alpha
    HR(dc->CreateSolidColorBrush(color,
        brush.GetAddressOf()));

    const wchar_t* text = L"Direct2D Sample";
    dc->DrawTextW(text, wcslen(text),
        m_textFormat.Get(),
        RectF(0.0F, 0.0F,
            400.0F, 300.0F),
        brush.Get());

    HR(surface->EndDraw());
}

void CreateDeviceResources(HWND hWnd,
    ComPtr<ID3D11Device>& m_device3D,
    ComPtr<IDCompositionDesktopDevice>& m_device,
    ComPtr<IDCompositionTarget>& m_target,
    ComPtr<IDWriteTextFormat>& m_textFormat,
    ComPtr<IDCompositionVisual2>& rootVisual)
{
    ASSERT(!IsDeviceCreated(m_device3D));

    CreateDevice3D(m_device3D);

    ComPtr<ID2D1Device> const device2D = CreateDevice2D(m_device3D);

    HR(DCompositionCreateDevice2(
        device2D.Get(),
        __uuidof(m_device),
        reinterpret_cast<void**>(m_device.ReleaseAndGetAddressOf())));

    HR(m_device->CreateTargetForHwnd(hWnd,
        true,
        m_target.ReleaseAndGetAddressOf()));

    CreateTextFormat(m_textFormat);

    rootVisual = CreateVisual(m_device);

    HR(m_target->SetRoot(rootVisual.Get()));

    //ComPtr<ID2D1DeviceContext> dc;

    //HR(device2D->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
    //    dc.GetAddressOf()));


    ComPtr<IDCompositionVisual2> frontVisual = CreateVisual(m_device);
    HR(frontVisual->SetOffsetX(0.0f));
    HR(frontVisual->SetOffsetY(0.0f));

    HR(rootVisual->AddVisual(frontVisual.Get(), false, nullptr));

    ComPtr<IDCompositionSurface> surface =
        CreateSurface(m_device,
            LogicalToPhysical(400, m_dpiX),
            LogicalToPhysical(300, m_dpiY));

    HR(rootVisual->SetContent(surface.Get()));

    ComPtr<IDCompositionSurface> frontSurface =
        CreateSurface(m_device,
            LogicalToPhysical(400, m_dpiX),
            LogicalToPhysical(300, m_dpiY));

    HR(frontVisual->SetContent(frontSurface.Get()));

    ComPtr<ID2D1DeviceContext> dc;
    POINT offset = {};

    HR(surface->BeginDraw(nullptr,
        __uuidof(dc),
        reinterpret_cast<void**>(dc.GetAddressOf()),
        &offset));
    
    dc->SetDpi(m_dpiX,
        m_dpiY);

    dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
        PhysicalToLogical(offset.y, m_dpiY)));
    // Draw something
    dc->Clear();

    HR(surface->EndDraw());

    DrawCardFront(m_textFormat,
        frontSurface);

    //HR(m_device->Commit());

    //HR(rootVisual->RemoveVisual(frontVisual.Get()));

    HR(m_device->Commit());
}

class  RoundButton
{
protected:
    RECTF rect;//Î»ÖÃ
    float defaultRadius; //dpi 96.0µÄ°ë¾¶
    float radius;   //°ë¾¶
    float diameter;   //Ö±¾¶

public:
    ComPtr<IDCompositionVisual2> ButtonVisual;
    ComPtr<IDCompositionVisual2> ButtonDownVisual;

    void CreateButton(ComPtr<IDCompositionDesktopDevice>& m_device,
       float left, float top,
        float diameter)
    {
        defaultRadius = (unsigned)diameter / 2;
        this->diameter = (unsigned)LogicalToPhysical(diameter, m_dpiX);
        radius = diameter / 2;

        rect.left = (unsigned)LogicalToPhysical(left, m_dpiX);
        rect.top = (unsigned)LogicalToPhysical(top, m_dpiY);
        rect.right = rect.left + this->diameter;
        rect.bottom = rect.top + this->diameter;

        ButtonVisual = CreateVisual(m_device);
        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        ButtonDownVisual = CreateVisual(m_device);

        DrawButton(m_device);
        DrawButtonDown(m_device);
    }
    
    
    bool IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
        {
            X -= rect.left;
            Y -= rect.top;
            if (radius >= sqrtf((radius - X) * (radius - X) + (radius - Y) * (radius - Y)))
                return TRUE;
            else
                return FALSE;
            return TRUE;
        }
        else
            return FALSE;
    }

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {

        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
        HR(m_device->Commit());
    }

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
        HR(m_device->Commit());
    }

    void ChangeDPI(ComPtr<IDCompositionDesktopDevice>& m_device,
        float left, float top)
    {
        diameter = (unsigned)LogicalToPhysical(defaultRadius * 2, m_dpiX);
        radius = diameter / 2;

        rect.left = (unsigned)LogicalToPhysical(left, m_dpiX);
        rect.top = (unsigned)LogicalToPhysical(top, m_dpiY);
        rect.right = rect.left + diameter;
        rect.bottom = rect.top + diameter;

        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        DrawButton(m_device);
        DrawButtonDown(m_device);
    }

    void Move(LONG left, LONG top)
    {
        rect.left = LogicalToPhysical(left, m_dpiX);
        rect.top = LogicalToPhysical(top, m_dpiY);;
        rect.right = left + diameter;
        rect.bottom = top + diameter;

        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        HR(ButtonDownVisual->SetOffsetX(rect.left));
        HR(ButtonDownVisual->SetOffsetY(rect.top));
    }

    virtual void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device) = 0;
    
    virtual void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device) = 0;

};

class  CloseButton : public RoundButton
{
public:

    bool IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
            return TRUE;
        else
            return FALSE;
    }

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {

        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
    }

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
    }

    void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonSurface =
            CreateSurface(m_device,
                diameter,
                diameter);

        HR(ButtonVisual->SetContent(ButtonSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpiX,
            m_dpiY);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
            PhysicalToLogical(offset.y, m_dpiY)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xFF4E5B,
            1.0f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ELLIPSE ellipse = Ellipse(Point2F(10, 10),
            10,
            10);
        dc->FillEllipse(ellipse,
            brush.Get());

        HR(ButtonSurface->EndDraw());
    }

    void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonDownSurface =
            CreateSurface(m_device,
                diameter,
                diameter);

        HR(ButtonDownVisual->SetContent(ButtonDownSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonDownSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpiX,
            m_dpiY);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
            PhysicalToLogical(offset.y, m_dpiY)));

        // Draw something
        dc->Clear();
        
        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0x8F000F,
            1.0F); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        dc->DrawLine(Point2F(6.0F, 6.0F),
            Point2F(14.0F, 14.0F),
            brush.Get(),
            2.0F);

        dc->DrawLine(Point2F(14, 6),
            Point2F(6, 14),
            brush.Get(),
            2.0F);

        HR(ButtonDownSurface->EndDraw());
    }

    LONG GetLeft()
    {
        return rect.left;
    }
    LONG GetTop()
    {
        return rect.top;
    }
};

class  MinButton : public RoundButton
{
public:

    bool IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
            return TRUE;
        else
            return FALSE;
    }

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {

        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
    }

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
    }

    void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonSurface =
            CreateSurface(m_device,
                diameter,
                diameter);

        HR(ButtonVisual->SetContent(ButtonSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpiX,
            m_dpiY);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
            PhysicalToLogical(offset.y, m_dpiY)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xFFB635,
            1.0f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ELLIPSE ellipse = Ellipse(Point2F(defaultRadius, defaultRadius),
            defaultRadius,
            defaultRadius);
        dc->FillEllipse(ellipse,
            brush.Get());

        HR(ButtonSurface->EndDraw());
    }

    void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonDownSurface =
            CreateSurface(m_device,
                diameter,
                diameter);

        HR(ButtonDownVisual->SetContent(ButtonDownSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonDownSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpiX,
            m_dpiY);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
            PhysicalToLogical(offset.y, m_dpiY)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xC26C16,
            1.0F); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        dc->DrawLine(Point2F(5, defaultRadius),
            Point2F(15, defaultRadius),
            brush.Get(),
            2.0F);

        HR(ButtonDownSurface->EndDraw());
    }

    LONG GetRight()
    {
        return rect.right;
    }
    LONG GetBottom()
    {
        return rect.bottom;
    }
};

class RoundedRectangleButton
{
private:
    RECTF defaultRect, rect;
    float defaultWidth, defaultHeight, width, height;
    float defaultDiameter, diameter;

    ComPtr<IDWriteTextFormat> m_textFormat;
public:
    ComPtr<IDCompositionVisual2> ButtonVisual;
    ComPtr<IDCompositionVisual2> ButtonDownVisual;
    ComPtr<IDCompositionVisual2> LabelVisual;

    void CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device,
        float left, float top,
        float width, float height,
        float diameter )
    {
        defaultWidth = width;
        defaultHeight = height;

        defaultRect.left = left;
        defaultRect.top = top;
        defaultRect.right = left + defaultWidth;
        defaultRect.bottom = top + defaultHeight;

        this->width = (unsigned)LogicalToPhysical(width, m_dpiX);
        this->height = (unsigned)LogicalToPhysical(height, m_dpiX);

        rect.left = (unsigned)LogicalToPhysical(defaultRect.left, m_dpiX);
        rect.top = (unsigned)LogicalToPhysical(defaultRect.top, m_dpiY);
        rect.right = rect.left + this->width;
        rect.bottom = rect.top + this->height;

        defaultDiameter = diameter;
        this->diameter = (unsigned)LogicalToPhysical(diameter, m_dpiX);

        ButtonVisual = CreateVisual(m_device);
        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        ButtonDownVisual = CreateVisual(m_device);

        ComPtr<IDWriteFactory2> factory;

            HR(DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(factory),
                reinterpret_cast<IUnknown**>(factory.GetAddressOf())));

            HR(factory->CreateTextFormat(L"Tahoma",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                20.0f,
                L"",
                m_textFormat.GetAddressOf()));

            HR(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
            HR(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER)); 
            
            DrawButton(m_device);
            DrawButtonDown(m_device);
    }
    
    bool IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
            return TRUE;
        else
            return FALSE;
    }

    void ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        width = (unsigned)LogicalToPhysical(defaultWidth, m_dpiX);
        height = (unsigned)LogicalToPhysical(defaultHeight, m_dpiX);

        rect.left = (unsigned)LogicalToPhysical(defaultRect.left, m_dpiX);
        rect.top = (unsigned)LogicalToPhysical(defaultRect.top, m_dpiY);
        rect.right = rect.left + width;
        rect.bottom = rect.top + height;

        diameter = (unsigned)LogicalToPhysical(defaultDiameter, m_dpiX);

        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        DrawButton(m_device);
        DrawButtonDown(m_device);
    }

    void DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonSurface =
            CreateSurface(m_device,
                width,
                height);

        HR(ButtonVisual->SetContent(ButtonSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpiX,
            m_dpiY);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
            PhysicalToLogical(offset.y, m_dpiY)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0x506350,
            0.5f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ROUNDED_RECT roundedRectangle = RoundedRect(RectF(1.0F, 1.0F, defaultWidth -1.0F, defaultHeight -1.0F),
            defaultDiameter , defaultDiameter );
        dc->DrawRoundedRectangle(roundedRectangle,
            brush.Get(),
            2.0F);

        ComPtr<ID2D1SolidColorBrush> brush1;
        D2D1_COLOR_F const color1 = ColorF(0xC3C3C3,
            0.5f); // alpha
        HR(dc->CreateSolidColorBrush(color1,
            brush1.GetAddressOf()));
        
        D2D1_ROUNDED_RECT roundedRectangle1 = RoundedRect(RectF(2.0F, 2.0F, defaultWidth - 2.0F, defaultHeight - 2.0F),
            defaultDiameter, defaultDiameter);
        dc->FillRoundedRectangle(roundedRectangle1, brush1.Get());

        HR(ButtonSurface->EndDraw());
        
    }

    void DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonDownSurface =
            CreateSurface(m_device,
                width,
                height);

        HR(ButtonDownVisual->SetContent(ButtonDownSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonDownSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpiX,
            m_dpiY);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpiX),
            PhysicalToLogical(offset.y, m_dpiY)));

        // Draw something
        dc->Clear();
        //0x506350
        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xC3C3C3,
            0.5f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ROUNDED_RECT roundedRectangle = RoundedRect(RectF(0.0F, 0.0F, defaultWidth, defaultHeight),
            defaultDiameter, defaultDiameter);
        dc->FillRoundedRectangle(roundedRectangle, brush.Get());

        HR(ButtonDownSurface->EndDraw());
    }

    void ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        
        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
        HR(m_device->Commit());
    }

    void ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
        HR(m_device->Commit());
    }
};

class Label
{

};