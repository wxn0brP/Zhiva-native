#include "webview.h"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <thread>
#include <cstdlib>

#if defined(_WIN32)
#include <windows.h>
#include <dwmapi.h>
#endif

#ifdef WEBVIEW_GTK
#include <gtk/gtk.h>
#endif

// This is a private API from webview.h to parse JSON arguments.
namespace webview {
namespace detail {
std::string json_parse(const std::string &s, const std::string &key,
                       const int index);
}
}

// --- Application Context ---
struct AppContext {
    webview_t w;
};

// --- Bound Functions ---

void set_window_title(const char *seq, const char *req, void *arg) {
    AppContext* context = static_cast<AppContext*>(arg);
    try {
        std::string title = webview::detail::json_parse(req, "", 0);
        webview_set_title(context->w, title.c_str());
    } catch (...) {
        std::cerr << "Error parsing title from JS" << std::endl;
    }
}

void is_app(const char *seq, const char *req, void *arg) {
    AppContext* context = static_cast<AppContext*>(arg);
    webview_return(context->w, seq, 0, "true");
}

void close_app(const char *seq, const char *req, void *arg) {
    AppContext* context = static_cast<AppContext*>(arg);
    webview_terminate(context->w);
}

void echo(const char *seq, const char *req, void *arg) {
    try {
        std::string text = webview::detail::json_parse(req, "", 0);
        std::cout << text << std::endl;
    } catch (...) {
        std::cerr << "Error parsing echo text from JS" << std::endl;
    }
}

void open_external(const char *seq, const char *req, void *arg) {
    std::string url;
    try {
        url = webview::detail::json_parse(req, "", 0);
    } catch (...) {
        std::cerr << "Error parsing url for open_external from JS" << std::endl;
        return;
    }

    if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
        std::cerr << "[Z-NTV-1-07] Invalid URL for external open: " << url << std::endl;
        return;
    }

    std::string cmd;
#if defined(_WIN32)
    cmd = "cmd /c start \"\" \"" + url + "\"";
#elif defined(__APPLE__)
    cmd = "open \"" + url + "\"";
#else // Assuming Linux/BSD
    cmd = "xdg-open \"" + url + "\"";
#endif
    int ret = system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "[Z-NTV-1-09] Error opening external URL: " << url << std::endl;
    }
}

// --- Main Application ---
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Z-NTV-1-05] Usage: %s <url_or_port> [title]\n", argv[0]);
        return 1;
    }
    
    std::string target = argv[1];
    if (target.find("://") == std::string::npos) {
        target = "http://localhost:" + target;
    }

    const char* title = "wxn0brP";
    if (argc > 2) {
        title = argv[2];
    }
    
    webview_t w = webview_create(1, nullptr);
    if (!w) {
        std::cerr << "Failed to create webview." << std::endl;
        return 1;
    }

    AppContext context = { w };

#if defined(_WIN32)
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
    if (hIcon) {
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

    webview_set_title(w, title);

    webview_bind(w, "zhiva_setWindowTitle", set_window_title, &context);
    webview_bind(w, "zhiva_isApp", is_app, &context);
    webview_bind(w, "zhiva_closeApp", close_app, &context);
    webview_bind(w, "zhiva_echo", echo, &context);
    webview_bind(w, "zhiva_openExternal", open_external, &context);

    webview_init(w, R"(
        document.addEventListener('DOMContentLoaded', () => {
            const titleObserver = new MutationObserver((mutations) => {
                mutations.forEach((mutation) => {
                    if (mutation.type === 'attributes' && mutation.attributeName === 'title' && mutation.target.title) {
                        window.zhiva_setWindowTitle(mutation.target.title);
                    }
                });
            });
            const titleElement = document.querySelector('title');
            if (titleElement) {
                titleObserver.observe(titleElement, { attributes: true });
                if (document.title) {
                    zhiva_setWindowTitle(document.title);
                }
            }
            
            document.addEventListener('click', (e) => {
                const anchor = e.target.closest('a');
                if (anchor && anchor.target === '_blank') {
                    e.preventDefault();
                    zhiva_openExternal(anchor.href);
                }
            }, true);

            window.addEventListener("message", (event) => {
                if (event.data && event.data.type === "open-link") {
                    zhiva_openExternal(event.data.url);
                }
            });
        });
    )");

    webview_navigate(w, target.c_str());

    webview_run(w);

    webview_destroy(w);
    return 0;
}