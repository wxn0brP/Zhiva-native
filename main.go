package main

import (
	"bufio"
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
	w.Bind("zhiva_echo", func(data string) { fmt.Println(data) })

	w.Bind("zhiva_openWindow", func(url string) {
		now := time.Now()

		if now.Sub(startTime) < 5*time.Second {
			fmt.Println("[Z-NTV-1-01] Opening new window blocked by 5s cooldown since start")
			return
		}

		if !queue.Add(now) {
			fmt.Println("[Z-NTV-1-02] Limit of queue reached or cooldown of 1s has not passed")
			return
		}

		exe, err := os.Executable()
		if err != nil {
			fmt.Println("[Z-NTV-1-03] Error:", err)
			return
		}
		cmd := exec.Command(exe, url)
		cmd.Start()
	})

	go func() {
		scanner := bufio.NewScanner(os.Stdin)
		for scanner.Scan() {
			line := scanner.Text()
			if !strings.Contains(line, "[JSON]") {
				continue
			}

			start := strings.Index(line, "[JSON]{")
			end := strings.Index(line, "}[/JSON]")
			if start == -1 || end == -1 || end <= start+6 {
				continue
			}

			jsonPayload := line[start+6 : end+1]

			w.Dispatch(func() {
			jsCode := fmt.Sprintf(
				"if (typeof zhiva_receive === 'function') zhiva_receive(%s, %s);",
				jsonPayload, "`"+jsonPayload+"`",
			)
			w.Eval(jsCode)
				w.Eval(jsCode)
			})
		}

		if err := scanner.Err(); err != nil {
			fmt.Fprintln(os.Stderr, "[Z-NTV-1-06] Error reading standard input:", err)
		}
	}()

	w.Navigate(target)

	time.AfterFunc(1*time.Second, func() {
		w.Dispatch(func() {
			pageTitle := w.GetPageTitle()
			if pageTitle != "" {
				w.SetTitle(pageTitle)
				fmt.Println("[Z-NTV-1-04] Title set to:", pageTitle)
			}
		})
	})

	w.Run()
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("[Z-NTV-1-05] Usage: %s <port>\n", os.Args[0])
		return
	}
	target := os.Args[1]

	name := "wxn0brP"
	if len(os.Args) > 2 {
		name = os.Args[2]
	}

	if !strings.Contains(target, "://") {
		target = "http://localhost:" + target
	}

	startTime := time.Now()
	queue := &WindowQueue{timestamps: []time.Time{}}

	createWindow(target, name, startTime, queue)
}