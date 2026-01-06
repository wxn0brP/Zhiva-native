//go:build !windows
// +build !windows

package main

import "github.com/lexesv/go-webview-gui"

func setDarkTitleBar(w webview.WebView) {
	// na Linux/macOS nie robimy nic
}
