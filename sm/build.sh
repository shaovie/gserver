GOPATH=`pwd`
gofmt -tabs=false -tabwidth=2 -w=true src
 
go install ism
#go build -o bin/ism ism
 
if ! [ -L bin/index ]; then
  cd bin && ln -sf ../html html; cd ../
fi
if ! [ -L bin/svc.json ]; then
  cd bin && ln -sf ../svc.json svc.json; cd ../
fi
echo 'finished'

#GOGCTRACE=1
