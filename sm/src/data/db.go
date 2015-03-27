package data

import (
  "database/sql"
  "fmt"

  "config"
  _ "mysql"
)

var dbInstance *sql.DB = nil

func GetDB() (*sql.DB, error) {
  if dbInstance != nil {
    return dbInstance, nil
  }
  dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s",
    config.Cfg.Db.User,
    config.Cfg.Db.Passwd,
    config.Cfg.Db.Host,
    config.Cfg.Db.Port,
    config.Cfg.Db.Name)
  dbInstance, err := sql.Open("mysql", dsn)
  if err != nil {
    return nil, err
  }
  return dbInstance, nil
}
