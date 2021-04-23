#pragma once
#include <objidl.h>
#include <GdiPlus.h>
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

// Ìí¼ÓÔ²½Ç¾ØÐÎÂ·¾¶
unsigned LogicalToPhysical(unsigned pixel,
    FLOAT const dpi, FLOAT const beforeDpi);

void AddRoundRectange(Gdiplus::GraphicsPath& path, float left, float top, float right, float bottom, float diameterX, float diameterY);

void gdiDrawShadow(Graphics& pGraphics);

void gdiPlusDraw(HWND hwnd);