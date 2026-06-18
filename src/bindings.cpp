#include "bindings.h"

#include "app_context.h"
#include "webview.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace webview
{
    namespace detail
    {
        std::string json_parse(const std::string &s, const std::string &key, const int index);
    }
}

void set_window_title(const char *seq, const char *req, void *arg)
{
    AppContext *context = static_cast<AppContext *>(arg);
    try
    {
        std::string title = webview::detail::json_parse(req, "", 0);
        webview_set_title(context->w, title.c_str());
    }
    catch (...)
    {
        std::cerr << "Error parsing title from JS" << std::endl;
    }
}

void is_app(const char *seq, const char *req, void *arg)
{
    AppContext *context = static_cast<AppContext *>(arg);
    webview_return(context->w, seq, 0, "true");
}

void close_app(const char *seq, const char *req, void *arg)
{
    AppContext *context = static_cast<AppContext *>(arg);
    webview_terminate(context->w);
}

void echo(const char *seq, const char *req, void *arg)
{
    try
    {
        std::string text = webview::detail::json_parse(req, "", 0);
        std::cout << text << std::endl;
    }
    catch (...)
    {
        std::cerr << "Error parsing echo text from JS" << std::endl;
    }
}

void open_external(const char *seq, const char *req, void *arg)
{
    std::string url;
    try
    {
        url = webview::detail::json_parse(req, "", 0);
    }
    catch (...)
    {
        std::cerr << "Error parsing url for open_external from JS" << std::endl;
        return;
    }

    if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0)
    {
        std::cerr << "[Z-NTV-1-07] Invalid URL for external open: " << url << std::endl;
        return;
    }

    std::string cmd;
#if defined(_WIN32)
    cmd = "cmd /c start \"\" \"" + url + "\"";
#elif defined(__APPLE__)
    cmd = "open \"" + url + "\"";
#else
    cmd = "xdg-open \"" + url + "\"";
#endif
    int ret = system(cmd.c_str());
    if (ret != 0)
    {
        std::cerr << "[Z-NTV-1-09] Error opening external URL: " << url << std::endl;
    }
}
