# Zhiva Native

This module is the core native engine of the Zhiva framework.

## Role in the Zhiva Project

The `native` module is the heart of a Zhiva desktop application. It is a standalone executable that creates and manages the native webview window. It acts as a bridge between the web-based UI (developed with `base-lib`) and the underlying operating system.

## Primary Responsibilities

-   **Window Management**: Creates a native window to host the web application.
-   **Webview Rendering**: Renders the web content using the system's webview engine.
-   **Native Bridge**: Exposes a communication channel that allows the JavaScript code to invoke native functionalities.

## Technology

The native engine is written in Go and utilizes the `go-webview-gui` library to create and manage the webview component.

## Vision

As the project evolves, this module will be expanded to provide a richer set of native APIs, enabling deeper integration with the host system. The goal is to keep the engine lightweight and cross-platform.
