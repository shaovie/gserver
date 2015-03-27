MOVE_PKG_TO_DIR="packages"
SVN_USER="bpublish"
SVN_PASSWD="78798324545"
PKG_EDITION="rc"
TMP_BUILD_DIR="2xf43df43"

# start
cd $(dirname $PWD/$0)/../

#svn up
svn update --username=$SVN_USER --password=$SVN_PASSWD --non-interactive --no-auth-cache -q
SVN_REVISION=`sed -n '4p' .svn/entries`

# build dir
if [ -d "$TMP_BUILD_DIR" ]; then
  rm -rf $TMP_BUILD_DIR
fi
mkdir $TMP_BUILD_DIR
mkdir $TMP_BUILD_DIR/bin
mkdir $TMP_BUILD_DIR/etc
mkdir $TMP_BUILD_DIR/libs

# build server
for svc in world/bin/iworld db_proxy/bin/db_proxy
do
  cd ${svc%%/*}
  make cleanall quiet=1;
  make publish=1 debug=1 quiet=1 svc_edition=$PKG_EDITION bin_v=$SVN_REVISION -j4
  cd ../
  if ! [ -f "$svc" ] ; then
    echo "compile $svc failed!"
    exit 1
  fi
  cp -f $svc $TMP_BUILD_DIR/bin
done

cp -rf config $TMP_BUILD_DIR
cp -f etc/open.sh etc/run.sh etc/svc.json etc/db.sql $TMP_BUILD_DIR/etc
cp -f libs/libicore.so libs/libjson.so libs/libmysqlclient.so.15 $TMP_BUILD_DIR/libs

# do package
cd $TMP_BUILD_DIR
find . -name "*.svn" | xargs rm -rf
PKG_NAME="yytx-$PKG_EDITION-svn_$SVN_REVISION-`date +%Y-%m-%d-%H-%M-%S`.tar.gz"
tar -zcf $PKG_NAME bin libs config etc
cd ../

# move
if ! [ -d "$MOVE_PKG_TO_DIR" ]; then
  mkdir $MOVE_PKG_TO_DIR
fi
cp $TMP_BUILD_DIR/$PKG_NAME $MOVE_PKG_TO_DIR
rm -rf $TMP_BUILD_DIR
ls -l $MOVE_PKG_TO_DIR/$PKG_NAME
