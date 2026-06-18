#pragma once

#include <string>

struct LaunchOptions
{
    std::string target;
    std::string title = "wxn0brP";
    std::string app_id;
    std::string app_host;
    std::string backend;
    bool backend_is_socket = false;
    std::string path = "/";
    bool use_app_scheme = false;
};

LaunchOptions parse_args(int argc, char *argv[]);
