#include "D2D.h"

extern unsigned WindowWidth;
extern unsigned WindowHeight;
extern unsigned WindowRoundD;
extern FLOAT m_dpi;


ComException::ComException(HRESULT const value) :
    result(value)
{}

void HR(HRESULT const result)
{
    if (S_OK != result)
    {
        throw ComException(result);
    }
}

void CreateDevice3D(ComPtr<ID3D11Device>& m_device3D)
{
    ASSERT(!IsDeviceCreated(m_device3D));


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

void CreateDevice2D(ComPtr<ID3D11Device>const& m_device3D, ComPtr<ID2D1Device>& device2D)
{
    ComPtr<IDXGIDevice3> deviceX;
    HR(m_device3D.As(&deviceX));

    D2D1_CREATION_PROPERTIES properties = {};
    #ifdef _DEBUG
    properties.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    #endif

    HR(D2D1CreateDevice(deviceX.Get(),
        properties,
        device2D.GetAddressOf()));
}

void CreateSurface(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionSurface>& surface,
    UINT width, UINT height)
{
    HR(m_device->CreateSurface(width,
        height,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_ALPHA_MODE_PREMULTIPLIED,
        surface.GetAddressOf()));
}

void CreateTextFormat(ComPtr<IDWriteTextFormat>& textFormat, const WCHAR* fontName, FLOAT fontSize)
{
    ComPtr<IDWriteFactory2> factory;

    HR(DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(factory),
        reinterpret_cast<IUnknown**>(factory.GetAddressOf())));

    HR(factory->CreateTextFormat(fontName, //L"Tahoma"
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        textFormat.GetAddressOf()));

    HR(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    HR(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
}

void ReleaseDeviceResources(ComPtr<ID3D11Device>& m_device3D)
{
    m_device3D.Reset();
}

void CreateVisual(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionVisual2>& visual)
{
    HR(m_device->CreateVisual(visual.GetAddressOf()));
}

void D2DDrawText(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionVisual2>const& visual,
    ComPtr<IDWriteTextFormat>const& textFormat,
    const WCHAR* string,
    const FLOAT left, const FLOAT top,
    const FLOAT right, const FLOAT bottom)
{
    ComPtr<IDCompositionSurface> surface;
    CreateSurface(m_device,
        surface,
        LogicalToPhysical(right, m_dpi),
        LogicalToPhysical(bottom, m_dpi));

    HR(visual->SetContent(surface.Get()));

    ComPtr<ID2D1DeviceContext> dc;
    POINT offset = {};

    HR(surface->BeginDraw(nullptr,
        __uuidof(dc),
        reinterpret_cast<void**>(dc.GetAddressOf()),
        &offset));

    dc->SetDpi(m_dpi,
        m_dpi);
    dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
        PhysicalToLogical(offset.y, m_dpi)));
    // Draw something
    dc->Clear();

    ComPtr<ID2D1SolidColorBrush> brush;
    D2D1_COLOR_F const color = ColorF(0.0f, // red
        0.0f, // green
        0.0f, // blue
        1.0f); // alpha
    HR(dc->CreateSolidColorBrush(color,
        brush.GetAddressOf()));

    dc->DrawTextW(string, wcslen(string),
        textFormat.Get(),
        RectF(left, top,
            right, bottom),
        brush.Get());

    HR(surface->EndDraw());
}

void CreateDeviceResources(HWND hWnd,
    ComPtr<ID3D11Device>& m_device3D,
    ComPtr<IDCompositionDesktopDevice>& m_device,
    ComPtr<IDCompositionTarget>& m_target,
    ComPtr<IDCompositionVisual2>& rootVisual)
{
    ASSERT(!IsDeviceCreated(m_device3D));

    CreateDevice3D(m_device3D);

    ComPtr<ID2D1Device> device2D;
    CreateDevice2D(m_device3D, device2D);

    HR(DCompositionCreateDevice2(
        device2D.Get(),
        __uuidof(m_device),
        reinterpret_cast<void**>(m_device.ReleaseAndGetAddressOf())));

    HR(m_device->CreateTargetForHwnd(hWnd,
        true,
        m_target.ReleaseAndGetAddressOf()));



    CreateVisual(m_device, rootVisual);

    HR(m_target->SetRoot(rootVisual.Get()));

    //ComPtr<ID2D1DeviceContext> dc;
    //HR(device2D->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
    //    dc.GetAddressOf()));
}

void WindowDraw(ComPtr<IDCompositionDesktopDevice>const& m_device,
    ComPtr<IDCompositionVisual2>const& rootVisual)
{
    ComPtr<IDCompositionSurface> surface;
    CreateSurface(m_device,
        surface,
        WindowWidth,
        WindowHeight);

    HR(rootVisual->SetContent(surface.Get()));

    ComPtr<ID2D1DeviceContext> dc;
    POINT offset = {};

    HR(surface->BeginDraw(nullptr,
        __uuidof(dc),
        reinterpret_cast<void**>(dc.GetAddressOf()),
        &offset));

    dc->SetDpi(96.0F,
        96.0F);

    dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, 96.0F),
        PhysicalToLogical(offset.y, 96.0F)));
    // Draw something
    dc->Clear();

    ComPtr<ID2D1SolidColorBrush> brush;
    D2D1_COLOR_F const color = ColorF(0xA0A0A0,
        0.6F); // alpha
    HR(dc->CreateSolidColorBrush(color,
        brush.GetAddressOf()));

    D2D1_ROUNDED_RECT roundedRectangle1 = RoundedRect(RectF(1.0F, 1.0F, WindowWidth - 2.0F, WindowHeight - 2.0F),
        WindowRoundD / 2, WindowRoundD / 2);
    dc->DrawRoundedRectangle(roundedRectangle1,
        brush.Get(),
        1.0F);

    HR(surface->EndDraw());

    HR(m_device->Commit());
}


    void RoundButton::CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device,
        float left, float top,
        float diameter)
    {
        defaultRadius = (unsigned)diameter / 2;
        this->diameter = (unsigned)LogicalToPhysical(diameter, m_dpi);
        radius = diameter / 2;

        rect.left = (unsigned)LogicalToPhysical(left, m_dpi);
        rect.top = (unsigned)LogicalToPhysical(top, m_dpi);
        rect.right = rect.left + this->diameter;
        rect.bottom = rect.top + this->diameter;

        CreateVisual(m_device, ButtonVisual);
        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        CreateVisual(m_device, ButtonDownVisual);

        DrawButton(m_device);
        DrawButtonDown(m_device);
    }


    bool RoundButton::IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
        {
            X -= rect.left;
            Y -= rect.top;
            if (radius * radius >= (radius - X) * (radius - X) + (radius - Y) * (radius - Y))
                return TRUE;
            else
                return FALSE;
            return TRUE;
        }
        else
            return FALSE;
    }

    void RoundButton::ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {

        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
        HR(m_device->Commit());
    }

    void RoundButton::ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
        HR(m_device->Commit());
    }

    void RoundButton::ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device,
        float left, float top)
    {
        diameter = (unsigned)LogicalToPhysical(defaultRadius * 2, m_dpi);
        radius = diameter / 2;

        rect.left = (unsigned)LogicalToPhysical(left, m_dpi);
        rect.top = (unsigned)LogicalToPhysical(top, m_dpi);
        rect.right = rect.left + diameter;
        rect.bottom = rect.top + diameter;

        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        DrawButton(m_device);
        DrawButtonDown(m_device);
    }

    void RoundButton::Move(LONG left, LONG top)
    {
        rect.left = LogicalToPhysical(left, m_dpi);
        rect.top = LogicalToPhysical(top, m_dpi);;
        rect.right = left + diameter;
        rect.bottom = top + diameter;

        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        HR(ButtonDownVisual->SetOffsetX(rect.left));
        HR(ButtonDownVisual->SetOffsetY(rect.top));
    }



    void CloseButton::CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        __super::CreateButton(m_device, 10.0F, 10.0F, 16.0F);
    }

    bool CloseButton::IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
            return TRUE;
        else
            return FALSE;
    }

    void CloseButton::ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {

        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
    }

    void CloseButton::ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
    }

    void CloseButton::DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonSurface;
        CreateSurface(m_device,
            ButtonSurface,
            diameter,
            diameter);

        HR(ButtonVisual->SetContent(ButtonSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpi,
            m_dpi);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
            PhysicalToLogical(offset.y, m_dpi)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xFF4E5B,
            1.0f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ELLIPSE ellipse = Ellipse(Point2F(8.0F, 8.0F),
            7.0F,
            7.0F);
        dc->FillEllipse(ellipse,
            brush.Get());

        ComPtr<ID2D1SolidColorBrush> brush1;
        D2D1_COLOR_F const color1 = ColorF(0xDF3C34,
            1.0f); // alpha
        HR(dc->CreateSolidColorBrush(color1,
            brush1.GetAddressOf()));

        D2D1_ELLIPSE ellipse1 = Ellipse(Point2F(8.0F, 8.0F),
            7.0F,
            7.0F);
        dc->DrawEllipse(ellipse1,
            brush1.Get(),
            1.0F);

        HR(ButtonSurface->EndDraw());
    }

    void CloseButton::DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonDownSurface;
        CreateSurface(m_device,
            ButtonDownSurface,
            diameter,
            diameter);

        HR(ButtonDownVisual->SetContent(ButtonDownSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonDownSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpi,
            m_dpi);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
            PhysicalToLogical(offset.y, m_dpi)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0x8F000F,
            1.0F); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        dc->DrawLine(Point2F(5.0F, 5.0F),
            Point2F(11.0F, 11.0F),
            brush.Get(),
            1.0F);

        dc->DrawLine(Point2F(11.0F, 5.0F),
            Point2F(5.0F, 11.0F),
            brush.Get(),
            1.0F);

        HR(ButtonDownSurface->EndDraw());
    }

    LONG CloseButton::GetLeft()
    {
        return rect.left;
    }
    LONG CloseButton::GetTop()
    {
        return rect.top;
    }

    void MinButton::CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        __super::CreateButton(m_device, 33.0F, 10.0F, 16.0F);
    }

    bool MinButton::IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
            return TRUE;
        else
            return FALSE;
    }

    void MinButton::ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {

        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
    }

    void MinButton::ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
    }

    void MinButton::DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonSurface;
        CreateSurface(m_device,
            ButtonSurface,
            diameter,
            diameter);

        HR(ButtonVisual->SetContent(ButtonSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpi,
            m_dpi);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
            PhysicalToLogical(offset.y, m_dpi)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xFFB635,
            1.0f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ELLIPSE ellipse = Ellipse(Point2F(8.0F, 8.0F),
            7.0F,
            7.0F);
        dc->FillEllipse(ellipse,
            brush.Get());

        ComPtr<ID2D1SolidColorBrush> brush1;
        D2D1_COLOR_F const color1 = ColorF(0xDE9A0A,
            1.0f); // alpha
        HR(dc->CreateSolidColorBrush(color1,
            brush1.GetAddressOf()));

        D2D1_ELLIPSE ellipse1 = Ellipse(Point2F(8.0F, 8.0F),
            7.0F,
            7.0F);
        dc->DrawEllipse(ellipse1,
            brush1.Get(),
            1.0F);

        HR(ButtonSurface->EndDraw());
    }

    void MinButton::DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonDownSurface;
        CreateSurface(m_device,
            ButtonDownSurface,
            diameter,
            diameter);

        HR(ButtonDownVisual->SetContent(ButtonDownSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonDownSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpi,
            m_dpi);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
            PhysicalToLogical(offset.y, m_dpi)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xC26C16,
            1.0F); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        dc->DrawLine(Point2F(4.0F, 8.0F),
            Point2F(12.0F, 8.0F),
            brush.Get(),
            2.0F);

        HR(ButtonDownSurface->EndDraw());
    }

    LONG MinButton::GetRight()
    {
        return rect.right;
    }
    LONG MinButton::GetBottom()
    {
        return rect.bottom;
    }

    void RoundedRectangleButton::CreateButton(ComPtr<IDCompositionDesktopDevice>const& m_device,
        FLOAT left, FLOAT top,
        FLOAT width, FLOAT height,
        FLOAT diameter)
    {
        defaultWidth = width;
        defaultHeight = height;

        defaultRect.left = left;
        defaultRect.top = top;
        defaultRect.right = left + defaultWidth;
        defaultRect.bottom = top + defaultHeight;

        this->width = (unsigned)LogicalToPhysical(width, m_dpi);
        this->height = (unsigned)LogicalToPhysical(height, m_dpi);

        rect.left = (unsigned)LogicalToPhysical(defaultRect.left, m_dpi);
        rect.top = (unsigned)LogicalToPhysical(defaultRect.top, m_dpi);
        rect.right = rect.left + this->width;
        rect.bottom = rect.top + this->height;

        defaultDiameter = diameter;
        this->diameter = (unsigned)LogicalToPhysical(diameter, m_dpi);

        CreateVisual(m_device, ButtonVisual);
        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        CreateVisual(m_device, textVisual);
        CreateVisual(m_device, ButtonDownVisual);

        CreateTextFormat(textFormat, L"Tahoma", 22.0F);

        DrawButton(m_device);

        D2DDrawText(m_device, textVisual, textFormat,
            L"OK",
            0.0F, 0.0F, defaultWidth, defaultHeight);

        DrawButtonDown(m_device);

        HR(ButtonVisual->AddVisual(textVisual.Get(), false, nullptr));
    }

    bool RoundedRectangleButton::IsMouseIn(LONG X, LONG Y)
    {
        if (X > rect.left && X < rect.right && Y > rect.top && Y < rect.bottom)
            return TRUE;
        else
            return FALSE;
    }

    void RoundedRectangleButton::ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        width = (unsigned)LogicalToPhysical(defaultWidth, m_dpi);
        height = (unsigned)LogicalToPhysical(defaultHeight, m_dpi);

        rect.left = (unsigned)LogicalToPhysical(defaultRect.left, m_dpi);
        rect.top = (unsigned)LogicalToPhysical(defaultRect.top, m_dpi);
        rect.right = rect.left + width;
        rect.bottom = rect.top + height;

        diameter = (unsigned)LogicalToPhysical(defaultDiameter, m_dpi);

        HR(ButtonVisual->SetOffsetX(rect.left));
        HR(ButtonVisual->SetOffsetY(rect.top));

        DrawButton(m_device);

        D2DDrawText(m_device, textVisual, textFormat,
            L"OK",
            0.0F, 0.0F, defaultWidth, defaultHeight);

        DrawButtonDown(m_device);
    }

    void RoundedRectangleButton::DrawButton(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonSurface;
        CreateSurface(m_device,
            ButtonSurface,
            width,
            height);

        HR(ButtonVisual->SetContent(ButtonSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpi,
            m_dpi);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
            PhysicalToLogical(offset.y, m_dpi)));

        // Draw something
        dc->Clear();

        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const color = ColorF(0xC3C3C3,
            0.5f); // alpha
        HR(dc->CreateSolidColorBrush(color,
            brush.GetAddressOf()));

        D2D1_ROUNDED_RECT roundedRectangle = RoundedRect(RectF(2.0F, 2.0F, defaultWidth - 2.0F, defaultHeight - 2.0F),
            diameter, diameter);
        dc->FillRoundedRectangle(roundedRectangle, brush.Get());

        ComPtr<ID2D1SolidColorBrush> brush1;
        D2D1_COLOR_F const color1 = ColorF(0x506350,
            0.5f); // alpha
        HR(dc->CreateSolidColorBrush(color1,
            brush1.GetAddressOf()));

        D2D1_ROUNDED_RECT roundedRectangle1 = RoundedRect(RectF(1.0F, 1.0F, defaultWidth - 1.0F, defaultHeight - 1.0F),
            defaultDiameter, defaultDiameter);
        dc->DrawRoundedRectangle(roundedRectangle1,
            brush1.Get(),
            2.0F);

        HR(ButtonSurface->EndDraw());

    }

    void RoundedRectangleButton::DrawButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        ComPtr<IDCompositionSurface> ButtonDownSurface;
        CreateSurface(m_device,
            ButtonDownSurface,
            width,
            height);

        HR(ButtonDownVisual->SetContent(ButtonDownSurface.Get()));

        ComPtr<ID2D1DeviceContext> dc;
        POINT offset = {};

        HR(ButtonDownSurface->BeginDraw(nullptr,
            __uuidof(dc),
            reinterpret_cast<void**>(dc.GetAddressOf()),
            &offset));

        dc->SetDpi(m_dpi,
            m_dpi);

        dc->SetTransform(Matrix3x2F::Translation(PhysicalToLogical(offset.x, m_dpi),
            PhysicalToLogical(offset.y, m_dpi)));

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

    void RoundedRectangleButton::ButtonDown(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(textVisual.Get()));
        HR(ButtonVisual->AddVisual(ButtonDownVisual.Get(), false, nullptr));
        HR(ButtonVisual->AddVisual(textVisual.Get(), false, nullptr));
        HR(m_device->Commit());
    }

    void RoundedRectangleButton::ButtonUp(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        HR(ButtonVisual->RemoveVisual(ButtonDownVisual.Get()));
        HR(m_device->Commit());
    }

    void Label::CreateLabel(ComPtr<IDCompositionDesktopDevice>const& m_device,
        const WCHAR* fontName,
        FLOAT fontsize,
        const WCHAR* string,
        FLOAT left, FLOAT top, FLOAT right, FLOAT bottom)
    {
        this->string = string;
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;

        CreateVisual(m_device, LabelVisual);

        CreateTextFormat(textFormat, fontName, fontsize);

        D2DDrawText(m_device,
            LabelVisual,
            textFormat,
            string,
            left, top, right, bottom);
    }
    void Label::ChangeDPI(ComPtr<IDCompositionDesktopDevice>const& m_device)
    {
        D2DDrawText(m_device,
            LabelVisual,
            textFormat,
            string,
            left, top, right, bottom);
    }