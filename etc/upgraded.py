# -*- coding: utf-8 -*-

import os
import sys
import json
import getopt
import urllib2
import hashlib
import time

#
game_root = sys.path[0] + "/../"

def get_file(file_name):
  try:
    if os.path.exists(file_name):
      os.unlink(file_name)
    os.system('sh %s/upgrade/fget.sh %s' % (game_root, file_name))
  except:
    sys.exit(1)

#
os.chdir(sys.path[0])
get_file("upgrade.json")
if not os.path.exists("upgrade.json"):
  sys.exit(1)

# check upgrade.json, avoid upgrade repead!
os.chdir(sys.path[0])
new_md5 = hashlib.new("md5", open("upgrade.json").read()).hexdigest()
if os.path.exists("upgrade.log"):
  if open("upgrade.log").read().find(new_md5) != -1:
    sys.exit(1)

# get json
os.chdir(sys.path[0])
upgrade_js = json.loads(open("upgrade.json", "r").read())

# check svc
find_svc = False
for s in upgrade_js["svc"]:
  svc_path = game_root + "/" + "s" + str(s) + "/svc.json"
  if os.path.exists(svc_path):
    if json.loads(open(svc_path, "r").read())["platform"] in upgrade_js["for"]:
      find_svc = True;
    break
if not find_svc:
  sys.exit(1)

# get package
os.chdir(sys.path[0])
if "upgrade_bin" in upgrade_js["ops"]:
  get_file(upgrade_js["package"])
  if not os.path.exists(upgrade_js["package"]):
    sys.exit(1)

  os.system("tar -zxf " + upgrade_js["package"] + " -C ../")
  os.unlink(upgrade_js["package"])

# Gm
if "gm" in upgrade_js["ops"]:
  if upgrade_js.has_key("gm_ctrl"):
    for s in upgrade_js["svc"]:
      if s[0] == 's':
        s = s[1:]
      svc_path = game_root + "/" + "s" + str(s) + "/svc.json"
      if os.path.exists(svc_path):
        url = "http://127.0.0.1:" + str(11000 + s) + "/" + upgrade_js["gm_ctrl"]
        try:
          s = urllib2.urlopen(url, timeout = 10).read()
          print("[%s] result is <%s>" % (url, s.rstrip()))
        except:
          print("call %s failed!" % (url))

# stop all
os.chdir(game_root)
for s in upgrade_js["svc"]:
  svc_path = game_root + "/" + "s" + str(s) + "/svc.json"
  if os.path.exists(svc_path):
    svc_js = json.loads(open(svc_path, "r").read())
    if svc_js["platform"] not in upgrade_js["for"]:
      continue

    if "stop" in upgrade_js["ops"]:
      os.system("sh %s/run.sh stop all" % ("s" + str(s)))

os.chdir(sys.path[0])
for s in upgrade_js["svc"]:
  svc_path = game_root + "/" + "s" + str(s) + "/svc.json"
  if os.path.exists(svc_path):
    svc_js = json.loads(open(svc_path, "r").read())
    if svc_js["platform"] not in upgrade_js["for"]:
      continue

    db_js = svc_js["db"]
    db_arg = " -P " + str(db_js["port"]) \
        + " -h " + db_js["host"] \
        + " -u " + db_js["user"] \
        + " -p" + db_js["passwd"] \
        + " " + db_js["db_name"]

    # backup db
    if "backup" in upgrade_js["ops"]:
      f = "s" + str(s) + "_v"
      for i in range(2, 0, -1):
        os.system("mv %s.tar.gz %s.tar.gz" %(f + str(i), f + str(i+1)))
      os.system("mv %s.tar.gz %s.tar.gz" %(f, f + str(1)))
      os.system(game_root + "/mysql/bin/mysqldump" + db_arg + " > " + f)
      os.system("tar -zcf %s.tar.gz %s" % (f, f))
      os.unlink(f);

    # upgrade db
    if "upgrade_db" in upgrade_js["ops"]:
      os.system(game_root + "/mysql/bin/mysql" + db_arg + " < " + game_root + "/etc/alter.sql")

    # start
    if "start" in upgrade_js["ops"]:
      time.sleep(2)
      os.system("sh ../%s/run.sh start all" % ("s" + str(s)))

open("upgrade.log", "a").write(new_md5 + "\n")
