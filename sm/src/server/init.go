package server

import (
  "encoding/json"
  "io/ioutil"
  "os"
  "time"

  "ilog"
)

type svcConfig struct {
  path string

  Host       string `json:"host"`
  Port       int    `json:"port"`
  MaxPayload int    `json:"max_payload"`
}

func (s *Server) loadSvcConfig() error {
  f, err := os.Open(s.sc.path)
  if err != nil {
    return err
  }
  defer f.Close()

  data, err := ioutil.ReadAll(f)
  if err != nil {
    return err
  }

  if err := json.Unmarshal(data, &s.sc); err != nil {
    return err
  }
  return nil
}
func (s *Server) initGameWorld() error {
  s.cltList = make([]*Client, s.sc.MaxPayload)

  if err := s.loadConfig(); err != nil {
    return nil
  }

  s.globalTimer()

  return nil
}

func (s *Server) loadConfig() error {
  ilog.Printf("load config ok")
  return nil
}

func (s *Server) globalTimer() {
  // 1 second interval
  go func() {
    ticker1 := time.NewTicker(1 * time.Second)
    ticker200Msec := time.NewTicker(200 * time.Millisecond)
    for {
      select {
      case <-ticker1.C:
        go func() {
          ilog.Printf("[%s]1 second ticker![%d clients]", time.Now().Format("2006-01-02 15:04:05.000"), len(s.cltList))
        }()
      case <-ticker200Msec.C:
        go func() {
          //ilog.Printf("[%s]200 msec ticker!", time.Now().Format("2006-01-02 15:04:05.000"))
        }()
      }
    }
  }()
}
