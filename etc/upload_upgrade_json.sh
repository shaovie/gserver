python -c 'import json; json.loads(open("/home/sparkcui/workspace/sg/etc/svc.json", "r").read())'
if [ "$?" == "0" ]; then
  cd ~
fi
