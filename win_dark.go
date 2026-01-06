//go:build windows
// +build windows

package main

import (
	"unsafe"
	"golang.org/x/sys/windows"
	"github.com/lexesv/go-webview-gui"
)

var (
	dwmapi                 = windows.NewLazySystemDLL("dwmapi.dll")
	procSetWindowAttribute = dwmapi.NewProc("DwmSetWindowAttribute")
)

const DWMWA_USE_IMMERSIVE_DARK_MODE = 20

func setDarkTitleBar(w webview.WebView) {
	hwnd := windows.Handle(w.WindowHandle())
	var value int32 = 1
	procSetWindowAttribute.Call(
		uintptr(hwnd),
		uintptr(DWMWA_USE_IMMERSIVE_DARK_MODE),
		uintptr(unsafe.Pointer(&value)),
		unsafe.Sizeof(value),
	)
}
