package main

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
	"sync"
	"time"

	webview "github.com/lexesv/go-webview-gui"
)

type WindowQueue struct {
	timestamps []time.Time
	lock       sync.Mutex
}

func (q *WindowQueue) Add(ts time.Time) bool {
	q.lock.Lock()
	defer q.lock.Unlock()

	cutoff := ts.Add(-1 * time.Second)
	newTimestamps := []time.Time{}
	for _, t := range q.timestamps {
		if t.After(cutoff) {
			newTimestamps = append(newTimestamps, t)
		}
	}
	q.timestamps = newTimestamps

	if len(q.timestamps) >= 3 {
		return false
	}

	q.timestamps = append(q.timestamps, ts)
	return true
}

func createWindow(target string, title string, startTime time.Time, queue *WindowQueue) {
	w := webview.New(true, true)
	defer w.Destroy()
	w.SetTitle(title)
	w.SetSize(1600, 900, webview.HintNone)
	w.Maximize()

	w.Bind("zhiva_setWindowTitle", func(newTitle string) {
		w.Dispatch(func() { w.SetTitle(newTitle) })
	})
	w.Bind("zhiva_isApp", func() bool { return true })
	w.Bind("zhiva_closeApp", func() { w.Dispatch(w.Terminate) })

	w.Bind("zhiva_openWindow", func(url string) {
		now := time.Now()

		if now.Sub(startTime) < 10*time.Second {
			fmt.Println("Opening new window blocked by 10s cooldown since start")
			return
		}

		if !queue.Add(now) {
			fmt.Println("Limit of queue reached or cooldown of 1s has not passed")
			return
		}

		exe, err := os.Executable()
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		cmd := exec.Command(exe, url)
		cmd.Start()
	})

	w.Navigate(target)
	w.Run()
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("Usage: %s <port>\n", os.Args[0])
		return
	}
	target := os.Args[1]

	if !strings.Contains(target, "://") {
		target = "http://localhost:" + target
	}

	startTime := time.Now()
	queue := &WindowQueue{timestamps: []time.Time{}}

	createWindow(target, "wxn0brP", startTime, queue)
}