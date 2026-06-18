#include "platform_window.h"

#if defined(_WIN32)
#include <dwmapi.h>
#include <windows.h>
#endif

#ifdef WEBVIEW_GTK
#include <gtk/gtk.h>
#endif

void apply_platform_window_defaults(webview_t w)
{
#if defined(_WIN32)
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
    if (hIcon)
    {
        SendMessage((HWND)webview_get_window(w), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage((HWND)webview_get_window(w), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }
    BOOL use_dark_mode = TRUE;
    DwmSetWindowAttribute((HWND)webview_get_window(w), 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

#ifdef WEBVIEW_GTK
    gtk_window_maximize(GTK_WINDOW(webview_get_window(w)));
#elif defined(_WIN32)
    ShowWindow((HWND)webview_get_window(w), SW_MAXIMIZE);
#else
    webview_set_size(w, 1600, 900, WEBVIEW_HINT_NONE);
#endif
}
