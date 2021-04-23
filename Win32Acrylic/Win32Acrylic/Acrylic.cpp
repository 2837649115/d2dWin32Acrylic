#include "Acrylic.h"

HMODULE hUser = GetModuleHandle(TEXT("user32.dll"));
pfnSetWindowCompositionAttribute setWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");

void SetWindowAcrylic(HWND hWnd,
	ACCENT_STATE Date,
	int cx, int cy,
	int b)
{
	if (setWindowCompositionAttribute)
	{

		ACCENT_POLICY accent{ Date,0, 0, 0 };

		// $AABBGGRR
		accent.GradientColor = 0x80DFDFDF;

		SetWindowRgn(hWnd, CreateRoundRectRgn(0, 0, cx, cy, b, b), FALSE);
		accent.AccentFlags = 0xff;   //clean

		WINDOWCOMPOSITIONATTRIBDATA data;
		data.Attrib = WCA_ACCENT_POLICY;
		data.pvData = &accent;
		data.cbData = sizeof(accent);

		setWindowCompositionAttribute(hWnd, &data);
	}
}

void OnWindowAcrylic(HWND hWnd, ACCENT_STATE Date)
{
	ACCENT_POLICY accent{ Date,0, 0, 0 };

	accent.GradientColor = 0x80DFDFDF;
	accent.AccentFlags = 0xff;

	WINDOWCOMPOSITIONATTRIBDATA data;
	data.Attrib = WCA_ACCENT_POLICY;
	data.pvData = &accent;
	data.cbData = sizeof(accent);

	setWindowCompositionAttribute(hWnd, &data);
}

void OffWindowAcrylic(HWND hWnd)
{
	ACCENT_POLICY accent{ ACCENT_DISABLED,0, 0, 0 };

	accent.GradientColor = 0x40F6F6F6;	//0x40FFFFFF
	accent.AccentFlags = 0xff;

	WINDOWCOMPOSITIONATTRIBDATA data;
	data.Attrib = WCA_ACCENT_POLICY;
	data.pvData = &accent;
	data.cbData = sizeof(accent);

	setWindowCompositionAttribute(hWnd, &data);
}