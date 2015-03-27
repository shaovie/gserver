package main

import (
  "flag"
  "fmt"
  "log"
  "os"
  "os/signal"
  "path"
  "strconv"
  "syscall"

  "config"
  "data"
  "ilog"
  "monitor"
  "prof"
  "web"
)

// Launch args
var (
  sConfigPath = ""
  showVersion = false
  toProfile   = false
)

func usage() {
  var usageStr = `
  Server options:
  -p                      To profile
  -c  FILE                Configuration file

  Common options:
  -h                      Show this message
  -v                      Show version
  `
  fmt.Printf("%s\n", usageStr)
  os.Exit(0)
}
func parseFlag() {
  flag.StringVar(&sConfigPath, "c", "svc.json", "Configuration file.")
  flag.BoolVar(&showVersion, "v", showVersion, "Show version.")
  flag.BoolVar(&toProfile, "p", toProfile, "To profile.")

  flag.Usage = usage
  flag.Parse()
}
func signalHandle(l *log.Logger) {
  ch := make(chan os.Signal)
  signal.Notify(ch, syscall.SIGHUP, syscall.SIGPIPE)
  for {
    _ = <-ch
  }
}
func outputPid() {
  pidPath := "pid"

  os.Mkdir(pidPath, 0755)
  _, name := path.Split(os.Args[0])
  pidPath += "/" + name
  pidf, err := os.OpenFile(pidPath, os.O_CREATE|os.O_WRONLY, 0644)
  if err != nil {
    log.Fatalf("Error - %s output pid [%s]", os.Args[0], err)
  }

  // Luanch only one instance.
  if ferr := syscall.Flock(int(pidf.Fd()), syscall.LOCK_EX|syscall.LOCK_NB); ferr != nil {
    log.Fatalf("Error - %s flock failed [%s]", os.Args[0], ferr)
  }
  pidf.Truncate(0)
  pidf.Write([]byte(strconv.Itoa(os.Getpid())))
}
func main() {
  parseFlag()

  if showVersion {
    fmt.Printf("version %s\n", "1.0.0")
    os.Exit(0)
  }

  //
  outputPid()

  l, err := ilog.InitLog("log")
  if err != nil {
    log.Fatalf("listen failed! [%s]", err.Error())
  }

  go signalHandle(l)

  if toProfile {
    prof.StartProf()
  }

  if err := config.Start(sConfigPath); err != nil {
    ilog.Printf("config start failed![%s]", err.Error())
  }

  if err := data.Start(); err != nil {
    ilog.Printf("data start failed![%s]", err.Error())
  }

  if err := monitor.Start(); err != nil {
    ilog.Printf("monitor start failed![%s]", err.Error())
  }

  if err := web.Start(); err != nil {
    ilog.Printf("web start failed![%s]", err.Error())
  }

  os.Exit(0)
}
