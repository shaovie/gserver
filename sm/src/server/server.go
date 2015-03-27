package server

import (
  "fmt"
  "net"

  "ilog"
)

type Server struct {
  sc       svcConfig
  listener *net.TCPListener
  cltList  []*Client
}

// New server
func New(cfgPath string) *Server {
  s := &Server{}
  s.sc.path = cfgPath

  return s
}

//
func (s *Server) Start() error {
  if err := s.loadSvcConfig(); err != nil {
    return err
  }

  // Init server (eg.. log, load config, global timer ...)
  if err := s.initGameWorld(); err != nil {
    return err
  }

  // Luanch net module
  if err := s.acceptLoop(); err != nil {
    return err
  }
  return nil
}

// Accept loop
func (s *Server) acceptLoop() error {
  addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf("%s:%d", s.sc.Host, s.sc.Port))
  if err != nil {
    return err
  }
  s.listener, err = net.ListenTCP("tcp", addr)
  if err != nil {
    return err
  }

  for {
    conn, err := s.listener.AcceptTCP()
    if err != nil {
      ilog.Printf("accept error![%s]", err.Error())
    } else {
      s.createClient(conn)
    }
  }
  return nil
}

// One go-runtime per client
func (s *Server) createClient(conn *net.TCPConn) error {
  ilog.Printf("new client %s", conn.RemoteAddr().String())

  c := newClient(s, conn)

  if c.Start() == nil {
    s.cltList = append(s.cltList, c)
  }

  return nil
}
