package prof

import (
  "log"
  "os"
  "runtime/pprof"
  "strconv"
  "time"
)

var (
  startTime time.Time
  pid       int
)

func StartProf() {
  startTime = time.Now()
  pid = os.Getpid()

  go startCPUProfile()
}

func startCPUProfile() {
  f, err := os.Create("cpu-" + strconv.Itoa(pid) + ".pprof")
  if err != nil {
    log.Fatal(err)
  }
  pprof.StartCPUProfile(f)
  go func() {
    t := time.NewTimer(10 * time.Second)
    <-t.C
    pprof.StopCPUProfile()
  }()
}
