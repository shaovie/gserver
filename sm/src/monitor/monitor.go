package monitor

import (
  "time"

  "ilog"
)

//
func Start() error {
  runTimer()
  return nil
}
func doCheckServerState() {
  ilog.Printf("[%s]1 second ticker!", time.Now().Format("2006-01-02 15:04:05.000"))
}
func doTestTimer() {
  ilog.Printf("[%s]200 msec ticker!", time.Now().Format("2006-01-02 15:04:05.000"))
}
func runTimer() {
  // 1 second interval
  go func() {
    ticker5 := time.NewTicker(5 * time.Second)
    ticker200Msec := time.NewTicker(200 * time.Millisecond)
    for {
      select {
      case <-ticker5.C:
        go func() {
          doCheckServerState()
        }()
      case <-ticker200Msec.C:
        go func() {
          doTestTimer()
        }()
      }
    }
  }()
}
