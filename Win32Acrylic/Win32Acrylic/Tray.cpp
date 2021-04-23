#include "Tray.h"

NOTIFYICONDATA nid;		//ÍÐÅÌÊôÐÔ
HMENU hMenu;			//ÍÐÅÌ²Ëµ¥

//ÊµÀý»¯ÍÐÅÌ
void InitTray(HINSTANCE hInstance, HWND hWnd)
{
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = IDI_TRAY;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
    lstrcpy(nid.szTip, APP_NAME);

    hMenu = CreatePopupMenu();//Éú³ÉÍÐÅÌ²Ëµ¥
    //ÎªÍÐÅÌ²Ëµ¥Ìí¼ÓÁ½¸öÑ¡Ïî
    AppendMenu(hMenu, MF_STRING, ID_SHOW, TEXT("ÌáÊ¾"));
    AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("ÍË³ö"));

    Shell_NotifyIcon(NIM_ADD, &nid);
}

//ÑÝÊ¾ÍÐÅÌÆøÅÝÌáÐÑ
void ShowTrayMsg()
{
    lstrcpy(nid.szInfoTitle, APP_NAME);
    lstrcpy(nid.szInfo, TEXT("ÒÑÒþ²Øµ½ÍÐÅÌ"));
    nid.uTimeout = 1000;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}


