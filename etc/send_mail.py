#!/bin/env python
#coding=utf-8

import os
import smtplib
import time
import socket
import sys
from email.mime.text import MIMEText

mail_to="sparkcui@cloudia.cn"

def get_ip():
  try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('www.baidu.com', 80))
    ip = s.getsockname()[0]
  except:
    ip = ""
  finally:
    s.close()
  return ip

def send_mail(to, sub, content):
  mail_host = "smtp.163.com"
  mail_user = "gserver_update"
  mail_pass = "update"
  me = mail_user + "<" + mail_user + "@163.com" + ">"

  msg = MIMEText(content, 'html')
  msg["Subject"] = sub
  msg["From"] = me
  msg["To"] = to

  try:
    s = smtplib.SMTP()
    s.connect(mail_host)
    s.login(mail_user, mail_pass)
    s.sendmail(me, to, msg.as_string())
    s.close()
    return True
  except:
    return False

if __name__ == '__main__':
  version = sys.argv[1]
  brief = sys.argv[2]
  content = sys.argv[3].replace(r'\n', '<br>')
  now = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
  subject = version + " [" + brief + "] [" + get_ip() + "] " + now
  send_mail(mail_to, subject, content)
