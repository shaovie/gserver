package ilog

import (
  "log"
  "os"
)

var iLog *log.Logger

func InitLog(logPath string) (*log.Logger, error) {
  os.Mkdir(logPath, 0755)
  lf, err := os.OpenFile(logPath+"/"+"log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
  if err != nil {
    return nil, err
  }
  iLog = log.New(lf, "", os.O_APPEND)
  return iLog, nil
}
func Printf(format string, v ...interface{}) {
  if iLog != nil {
    iLog.Printf(format, v...)
  }
}
