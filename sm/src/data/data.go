package data

import (
  "ilog"
)

// game server info
type GSInfo struct {
  Id       int
  Name     string
  GroupId  string
  Ip       string
  Port     int
  State    int
  OlPlayer int
  OpenTime int
}

func Start() error {
  _, err := GetDB()
  if err != nil {
    return err
  }
  return nil
}
func GetGSList() (*map[int]GSInfo, error) {
  db, err := GetDB()
  if err != nil {
    return nil, err
  }

  rows, err := db.Query("select * from gs_info")
  if err != nil {
    return nil, err
  }
  defer rows.Close()

  gslist := make(map[int]GSInfo)
  var gi GSInfo
  for rows.Next() {
    err = rows.Scan(&gi.Id, &gi.Name, &gi.GroupId, &gi.Ip, &gi.Port, &gi.State, &gi.OlPlayer, &gi.OpenTime)
    if err == nil {
      gslist[gi.Id] = gi
      ilog.Printf("insert %d %s", gi.Id, gi.Name)
    }
  }

  return &gslist, nil
}
