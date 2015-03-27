package web

import (
  "fmt"
  "html/template"
  "net/http"

  "config"
  "data"
)

func Start() error {
  http.HandleFunc("/", index)

  addr := fmt.Sprintf(":%d", config.Cfg.ListenPort)
  return http.ListenAndServe(addr, nil)
}
func index(w http.ResponseWriter, r *http.Request) {
  t, err := template.ParseFiles("html/index.html")
  if err != nil {
    http.Error(w, err.Error(), http.StatusInternalServerError)
    return
  }
  locals := make(map[string]interface{})
  gslist, e := data.GetGSList()
  if e != nil {
    http.Error(w, e.Error(), http.StatusInternalServerError)
    return
  }
  locals["gslist"] = gslist
  t.Execute(w, locals)
}
