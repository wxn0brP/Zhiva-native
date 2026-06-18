#include "webview.h"

#include "app_context.h"
#include "bindings.h"
#include "init_script.h"
#include "launch_options.h"
#include "platform_window.h"
#include "zhiva_scheme.h"

#include <cstdio>
#include <iostream>
#include <string>

namespace
{
    std::string backend_url_with_path(const std::string &backend, const std::string &path)
    {
        std::string target = backend;
        if (!target.empty() && target.back() == '/')
        {
            target.pop_back();
        }
        target += path.empty() || path.front() != '/' ? "/" + path : path;
        return target;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "[Z-NTV-1-05] Usage: %s <url_or_port> [title]\n", argv[0]);
        return 1;
    }

    LaunchOptions opts = parse_args(argc, argv);
    if (opts.use_app_scheme && (opts.app_id.empty() || opts.backend.empty()))
    {
        std::cerr << "[Z-NTV-1-11] Missing --app-id or backend." << std::endl;
        return 1;
    }

    webview_t w = webview_create(1, nullptr);
    if (!w)
    {
        std::cerr << "Failed to create webview." << std::endl;
        return 1;
    }

    AppContext context = {w, opts.app_id, opts.app_host, opts.backend, opts.backend_is_socket};

    apply_platform_window_defaults(w);
    webview_set_title(w, opts.title.c_str());

    webview_bind(w, "zhiva_setWindowTitle", set_window_title, &context);
    webview_bind(w, "zhiva_isApp", is_app, &context);
    webview_bind(w, "zhiva_closeApp", close_app, &context);
    webview_bind(w, "zhiva_echo", echo, &context);
    webview_bind(w, "zhiva_openExternal", open_external, &context);

#ifdef WEBVIEW_GTK
    if (opts.use_app_scheme)
    {
        register_zhiva_scheme(w, &context);
    }
#else
    if (opts.use_app_scheme)
    {
        std::cerr << "[Z-NTV-1-12] Stable app origin is not supported on this platform yet. Using backend URL." << std::endl;
        if (opts.backend_is_socket)
        {
            std::cerr << "[Z-NTV-1-13] Backend socket is not supported on this platform." << std::endl;
            return 1;
        }
        opts.target = backend_url_with_path(opts.backend, opts.path);
    }
#endif

    webview_init(w, zhiva_init_script());
    webview_navigate(w, opts.target.c_str());

    webview_run(w);

    webview_destroy(w);
    return 0;
}
