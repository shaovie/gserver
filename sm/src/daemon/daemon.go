package daemon

import (
  "log"
  "os"
  "os/signal"
  "path"
  "strconv"
  "syscall"
  "time"
)

var (
  isChildArg = "ischild"
  childPid   int
)

func sTime() string {
  return time.Now().Format("2006-01-02 15:04:05")
}

func toGuardMode(args []string) {
  _, pName := path.Split(os.Args[0])

  for {
    pid, err := syscall.ForkExec(args[0], args, nil)
    if err != nil {
      log.Fatalf("Error - %s fork failed! [%s]", pName, err.Error())
      os.Exit(1)
    }

    childPid = pid

    var ws syscall.WaitStatus
    _, err = syscall.Wait4(childPid, &ws, 0, nil)
    for err == syscall.EINTR {
      _, err = syscall.Wait4(childPid, &ws, 0, nil)
    }

    // Log
    lf, fErr := os.OpenFile("debug.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
    if fErr != nil {
      log.Fatalf("Error - open debug.log failed! [%s]", fErr.Error())
    }
    defer lf.Close()

    l := log.New(lf, "", os.O_APPEND)

    if ws.Exited() {
      l.Printf("[%s] Exited - %s exit status %d", sTime(), pName, ws.ExitStatus())
      os.Exit(1)
    }
    if ws.Signaled() {
      l.Printf("[%s] Exited - %s catch signal %d", sTime(), pName, ws.Signal())
    }

    time.Sleep(time.Second * 2) // After release os resouce
  }
}

func OutputPid(pidPath string) {
  if len(pidPath) == 0 {
    return
  }
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

func ToGuardMode(pidPath string) {
  isChild := false
  for _, arg := range os.Args {
    if arg == isChildArg {
      isChild = true
      break
    }
  }
  if !isChild {
    // Handle signal
    go masterSignalHandle()

    OutputPid(pidPath)

    toGuardMode(append(os.Args, isChildArg))
  }
}

func masterSignalHandle() {
  ch := make(chan os.Signal)
  signal.Notify(ch)
  for {
    sig := <-ch
    if sig == syscall.SIGUSR1 && childPid != 0 {
      syscall.Kill(childPid, syscall.SIGUSR1)
      os.Exit(0)
    }
  }
}

func ChildSignalHandle(l *log.Logger) {
  ch := make(chan os.Signal)
  signal.Notify(ch, syscall.SIGUSR1)
  for {
    sig := <-ch
    if sig == syscall.SIGUSR1 {
      // Do something ...
      time.Sleep(time.Second * 5)
      os.Exit(0)
    }
  }
}

func CleanAllFds() error {
  var rlimit syscall.Rlimit
  err := syscall.Getrlimit(syscall.RLIMIT_NOFILE, &rlimit)
  if err != nil {
    return err
  }

  cur := int(rlimit.Cur)
  for i := 0; i < cur; i++ {
    syscall.Close(i)
  }
  return nil
}
