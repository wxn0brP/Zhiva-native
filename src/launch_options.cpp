#include "launch_options.h"

#include <cstdint>
#include <string>

namespace
{
    bool has_scheme(const std::string &value)
    {
        return value.find("://") != std::string::npos;
    }

    void set_backend(LaunchOptions *opts, const std::string &backend)
    {
        opts->backend_is_socket = !backend.empty() && backend.front() == '/';
        if (opts->backend_is_socket || has_scheme(backend))
        {
            opts->backend = backend;
            return;
        }

        opts->backend = "http://localhost:" + backend;
    }
}

LaunchOptions parse_args(int argc, char *argv[])
{
    LaunchOptions opts;
    if (argc < 2)
    {
        return opts;
    }

    std::string first = argv[1];
    if (first != "--app-id")
    {
        opts.target = first;
        if (!has_scheme(opts.target))
        {
            opts.target = "http://localhost:" + opts.target;
        }
        if (argc > 2)
        {
            opts.title = argv[2];
        }
        return opts;
    }

    opts.use_app_scheme = true;
    for (int i = 1; i < argc; ++i)
    {
        std::string key = argv[i];
        if (key == "--app-id" && i + 1 < argc)
        {
            opts.app_id = argv[++i];
        }
        else if (key == "--backend" && i + 1 < argc)
        {
            set_backend(&opts, argv[++i]);
        }
        else if (key == "--path" && i + 1 < argc)
        {
            opts.path = argv[++i];
        }
        else if (i == argc - 1)
        {
            opts.title = key;
        }
    }

    if (opts.path.empty() || opts.path.front() != '/')
    {
        opts.path = "/" + opts.path;
    }
    opts.app_host = opts.app_id;
    opts.target = "zhiva-app://" + opts.app_host + opts.path;
    return opts;
}
