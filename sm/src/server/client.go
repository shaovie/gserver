package server

import (
  "net"
)

const (
  defaultBufSize = 1024
)

type Client struct {
  conn *net.TCPConn
  srv  *Server
}

func newClient(s *Server, conn *net.TCPConn) *Client {
  c := &Client{
    srv:  s,
    conn: conn,
  }
  return c
}

func (c *Client) Start() error {
  go c.clientLoop()
  return nil
}

func (c *Client) clientLoop() {

  recvBuf := make([]byte, defaultBufSize)

  //c.conn.SetReadBuffer(2323434)
  for {
    _, err := c.conn.Read(recvBuf)
    if err != nil {
      c.closeClient()
      return
    }
  }
}
func (c *Client) closeClient() {
  c.conn.Close()
  c.conn = nil
}
