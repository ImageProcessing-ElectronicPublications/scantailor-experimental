#!/bin/bash

if [ "$#" -lt 3 ]
then
  echo "Usage:  makeapp.sh <dest_dir> <scantailor_source_dir> <build_dir>"
  echo " "
  echo "dest_dir is where the output files are created."
  echo "scantailor_source_dir is where the scantailor-experimental source files are."
  echo "build_dir is the working directory where dependency libs are built."
  echo ""
  exit 0
fi

DESTDIR=$1
SRCDIR=$2
BUILDDIR=$3

export APP=$DESTDIR/ScanTailor-Experimental.app
export APPC=$APP/Contents
export APPM=$APPC/MacOS
export APPR=$APPC/Resources
export APPF=$APPC/Frameworks

rm -rf $APP
mkdir -p $APPC
mkdir -p $APPM
mkdir -p $APPR
mkdir -p $APPF

cp $SRCDIR/packaging/osx/ScanTailor-Experimental.icns $APPR
cp $SRCDIR/scantailor-experimental_*.qm $APPR
cp $SRCDIR/scantailor-experimental $APPM/ScanTailor-Experimental

stver=`cat version.h | grep 'VERSION "' | cut -d ' ' -f 3 | tr -d '"'`
cat $SRCDIR/packaging/osx/Info.plist.in | sed "s/@VERSION@/$stver/" >  $APPC/Info.plist

otool -L $APPM/ScanTailor-Experimental | tail -n +2 | tr -d '\t' | cut -f 1 -d ' ' | while read line; do
  case $line in
    $BUILDDIR/*)
      ourlib=`basename $line`
      cp $line $APPF >/dev/null 2>&1
      install_name_tool -change $line @executable_path/../Frameworks/$ourlib $APPM/ScanTailor-Experimental
      install_name_tool -id @executable_path/../Frameworks/$ourlib $APPF/$ourlib
      ;;
    esac
done

rm -rf ScanTailor.dmg $DESTDIR/ScanTailor-Experimental-$stver.dmg
cd $DESTDIR
macdeployqt $DESTDIR/ScanTailor-Experimental.app -dmg >/dev/null 2>&1
mv ScanTailor-Experimental.dmg $DESTDIR/ScanTailor-Experimental-$stver.dmg

