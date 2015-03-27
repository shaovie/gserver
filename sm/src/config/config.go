package config

import (
  "encoding/json"
  "io/ioutil"
  "os"
)

type dbConfig struct {
  Host   string `json:"host"`
  Port   int    `json:"port"`
  Name   string `json:"name"`
  User   string `json:"user"`
  Passwd string `json:"passwd"`
}
type svcConfig struct {
  Db         dbConfig `json:"db"`
  ListenPort int      `json:"port"`
}

var Cfg svcConfig

func Start(path string) error {
  f, err := os.Open(path)
  if err != nil {
    return err
  }
  defer f.Close()

  data, err := ioutil.ReadAll(f)
  if err != nil {
    return err
  }

  if err := json.Unmarshal(data, &Cfg); err != nil {
    return err
  }
  return nil
}
