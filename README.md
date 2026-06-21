# Zhiva Native

Native runtime for Zhiva desktop applications.

## Build locally

```sh
make build
```

Output:

```text
build/bin/zhiva
```

## Usage

```sh
zhiva <url_or_port> [title]
```

```text
url_or_port:
  https://example.com   -> https://example.com
  5173                  -> http://localhost:5173

title:
  optional window title
```

## App id variant

```sh
zhiva --app-id <id> --backend <url_or_port_or_socket> [--path <path>] [title]
```

```text
app origin:
  zhiva-app://<id>/<path>

backend:
  https://example.com   -> https://example.com
  5173                  -> http://localhost:5173
  /tmp/zhiva/app.sock   -> Unix socket

path:
  default: /
```

## License

MIT
