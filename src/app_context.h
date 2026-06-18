#pragma once

#include "webview.h"

#include <string>

struct AppContext
{
    webview_t w;
    std::string app_id;
    std::string app_host;
    std::string backend;
    bool backend_is_socket;
};
