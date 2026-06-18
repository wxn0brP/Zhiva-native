#include "zhiva_scheme.h"

#ifdef WEBVIEW_GTK

#include "webview.h"

#include <gio/gunixsocketaddress.h>
#include <iostream>
#include <libsoup/soup.h>
#include <string>
#include <webkit2/webkit2.h>

namespace
{

    void finish_scheme_error(WebKitURISchemeRequest *request, const std::string &message)
    {
        GError *error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED, message.c_str());
        webkit_uri_scheme_request_finish_error(request, error);
        g_error_free(error);
    }

    std::string backend_url_for_request(AppContext *context, WebKitURISchemeRequest *request)
    {
        if (!context || context->backend.empty())
        {
            return "";
        }

        const gchar *uri_raw = webkit_uri_scheme_request_get_uri(request);
        GError *error = nullptr;
        GUri *uri = g_uri_parse(uri_raw, G_URI_FLAGS_NONE, &error);
        if (!uri)
        {
            if (error)
            {
                g_error_free(error);
            }
            return "";
        }

        const gchar *path = g_uri_get_path(uri);
        const gchar *query = g_uri_get_query(uri);
        std::string target = context->backend_is_socket ? "http://zhiva.local" : context->backend;
        if (!target.empty() && target.back() == '/')
        {
            target.pop_back();
        }
        target += (path && *path) ? path : "/";
        if (query && *query)
        {
            target += "?";
            target += query;
        }
        g_uri_unref(uri);
        return target;
    }

    bool is_hop_by_hop_request_header(const char *name)
    {
        return g_ascii_strcasecmp(name, "Host") == 0 ||
               g_ascii_strcasecmp(name, "Connection") == 0 ||
               g_ascii_strcasecmp(name, "Content-Length") == 0 ||
               g_ascii_strcasecmp(name, "Keep-Alive") == 0 ||
               g_ascii_strcasecmp(name, "Proxy-Authenticate") == 0 ||
               g_ascii_strcasecmp(name, "Proxy-Authorization") == 0 ||
               g_ascii_strcasecmp(name, "TE") == 0 ||
               g_ascii_strcasecmp(name, "Trailer") == 0 ||
               g_ascii_strcasecmp(name, "Transfer-Encoding") == 0 ||
               g_ascii_strcasecmp(name, "Upgrade") == 0;
    }

    void copy_request_headers(SoupMessageHeaders *from, SoupMessageHeaders *to)
    {
        SoupMessageHeadersIter iter;
        const char *name = nullptr;
        const char *value = nullptr;
        soup_message_headers_iter_init(&iter, from);
        while (soup_message_headers_iter_next(&iter, &name, &value))
        {
            if (is_hop_by_hop_request_header(name))
            {
                continue;
            }
            soup_message_headers_append(to, name, value);
        }
    }

    void zhiva_scheme_callback(WebKitURISchemeRequest *request, gpointer user_data)
    {
        AppContext *context = static_cast<AppContext *>(user_data);
        if (!context || context->backend.empty())
        {
            finish_scheme_error(request, "Zhiva backend is not configured");
            return;
        }

        std::string target = backend_url_for_request(context, request);
        if (target.empty())
        {
            finish_scheme_error(request, "Invalid Zhiva app URI");
            return;
        }

        const gchar *method = webkit_uri_scheme_request_get_http_method(request);
        SoupMessage *message = soup_message_new(method && *method ? method : "GET", target.c_str());
        if (!message)
        {
            finish_scheme_error(request, "Failed to create backend request");
            return;
        }

        SoupMessageHeaders *source_headers = webkit_uri_scheme_request_get_http_headers(request);
        SoupMessageHeaders *request_headers = soup_message_get_request_headers(message);
        if (source_headers)
        {
            copy_request_headers(source_headers, request_headers);
        }

        GInputStream *body = webkit_uri_scheme_request_get_http_body(request);
        if (body)
        {
            const char *content_type = source_headers
                                           ? soup_message_headers_get_content_type(source_headers, nullptr)
                                           : nullptr;
            goffset content_length = source_headers
                                         ? soup_message_headers_get_content_length(source_headers)
                                         : -1;
            soup_message_set_request_body(message, content_type, body, content_length);
        }

        GError *error = nullptr;
        SoupSession *session = nullptr;
        if (context->backend_is_socket)
        {
            GSocketConnectable *connectable = G_SOCKET_CONNECTABLE(
                g_unix_socket_address_new(context->backend.c_str()));
            session = soup_session_new_with_options(
                "remote-connectable", connectable,
                nullptr);
            g_object_unref(connectable);
        }
        else
        {
            session = soup_session_new();
        }
        GBytes *bytes = soup_session_send_and_read(session, message, nullptr, &error);
        if (!bytes)
        {
            std::string error_message = error ? error->message : "Backend request failed";
            if (error)
            {
                g_error_free(error);
            }
            g_object_unref(session);
            g_object_unref(message);
            finish_scheme_error(request, error_message);
            return;
        }

        gsize size = 0;
        g_bytes_get_data(bytes, &size);
        GInputStream *stream = g_memory_input_stream_new_from_bytes(bytes);
        WebKitURISchemeResponse *response = webkit_uri_scheme_response_new(stream, static_cast<gint64>(size));
        SoupStatus status = soup_message_get_status(message);
        webkit_uri_scheme_response_set_status(
            response,
            static_cast<guint>(status),
            soup_message_get_reason_phrase(message));

        SoupMessageHeaders *response_headers = soup_message_get_response_headers(message);
        const char *content_type = soup_message_headers_get_content_type(response_headers, nullptr);
        if (content_type)
        {
            webkit_uri_scheme_response_set_content_type(response, content_type);
        }
        webkit_uri_scheme_request_finish_with_response(request, response);

        g_object_unref(response);
        g_object_unref(stream);
        g_bytes_unref(bytes);
        g_object_unref(session);
        g_object_unref(message);
    }

}

void register_zhiva_scheme(webview_t w, AppContext *context)
{
    auto *webview = static_cast<WebKitWebView *>(
        webview_get_native_handle(w, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER));
    if (!webview)
    {
        std::cerr << "[Z-NTV-1-10] Failed to get WebKit webview handle." << std::endl;
        return;
    }

    WebKitWebContext *web_context = webkit_web_view_get_context(webview);
    WebKitSecurityManager *security_manager = webkit_web_context_get_security_manager(web_context);
    webkit_security_manager_register_uri_scheme_as_secure(security_manager, "zhiva-app");
    webkit_security_manager_register_uri_scheme_as_cors_enabled(security_manager, "zhiva-app");
    webkit_web_context_register_uri_scheme(web_context, "zhiva-app", zhiva_scheme_callback, context, nullptr);
}

#endif
