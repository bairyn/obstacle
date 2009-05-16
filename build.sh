#!/bin/sh

cd `dirname $0`

# vars

build_serverdir=~/.tremulousoc/oc
build_webdir=/var/www/oc
build_pakname=${1}
build_pakonly=0

# make pak

if [ $# -lt 1 ]; then
	echo -ne "./build.sh <pakname>.pk3 (serverdir) (webdir) (pakonly)\nExample: ./build.sh data-oc-2.0-dev ~/.tremulousoc/oc /var/www\nExample: ./build.sh data-oc-2.0-dev . . yes\n(The second argument and the third agument, both '.', in the second example will be used by the script because pakonly is set)\n"
	exit 0
fi

if [ $# -ge 2 ]; then
	build_serverdir=${2}
fi

if [ $# -ge 3 ]; then
	build_webdir=${3}
fi

if [ $# -ge 4 ]; then
	if [ "$4" == "y" ] || [ "$4" == "-y" ] || [ "$4" == "yes" ] || [ "$4" == "--yes" ] || [ "$4" == "-yes" ] || [ "$4" == "1" ] || [ "$4" == "true" ]; then
		build_pakonly=1
	fi
fi

if ! make; then
	exit 1
fi

if ! make debug; then
	exit 1
fi

if [ -d "pak_tmp" ]; then
	if ! rm -r pak_tmp; then
		exit 1
	fi
fi

if ! mkdir ./pak_tmp; then
	exit 1
fi

if ! mkdir ./pak_tmp/vm; then
	exit 1
fi

if ! cp -R ./configs ./pak_tmp/configs; then
	exit 1
fi

if ! cp -R ./ui ./pak_tmp/ui; then
	exit 1
fi

if ! cp -R ./scripts ./pak_tmp/scripts; then
	exit 1
fi

if ! cp -R ./sound ./pak_tmp/sound; then
	exit 1
fi

if ! cp -R ./models ./pak_tmp/models; then
	exit 1
fi

if ! cp -R ./emoticons ./pak_tmp/emoticons; then
	exit 1
fi

if ! cp -R ./gfx ./pak_tmp/gfx; then
	exit 1
fi

if ! cp -R ./armour ./pak_tmp/armour; then
	exit 1
fi

if ! cp -R ./fonts ./pak_tmp/fonts; then
	exit 1
fi

if ! cp ./GPL ./pak_tmp/GPL; then
	exit 1
fi

if ! cp ./README ./pak_tmp/README; then
	exit 1
fi

if ! cp ./build/release-linux-x86/base/vm/cgame.qvm ./pak_tmp/vm/; then
	exit 1
fi

if ! cp ./build/release-linux-x86/base/vm/ui.qvm ./pak_tmp/vm/; then
	exit 1
fi

if [ -e ${build_pakname}.pk3 ]; then
	if ! rm ${build_pakname}.pk3; then
		exit 1
	fi
fi

if ! cd pak_tmp/; then
	exit 1
fi

if ! zip -9r ../${build_pakname}.pk3 vm/cgame.qvm vm/ui.qvm ui configs scripts sound models emoticons gfx armour fonts GPL; then
	exit 1
fi

if ! cd ../; then
	exit 1
fi

# copy into server

if ! cp build/debug-linux-x86/base/gamex86.so ${build_serverdir}/; then
	exit 1
fi

if ! cp ${build_pakname}.pk3 ${build_webdir}/; then
	exit 1
fi

if ! cp ${build_pakname}.pk3 ${build_serverdir}/; then
	exit 1
fi

# sometimes the binary copied can already be running
cp build/debug-linux-x86/tremded.x86 ${build_serverdir}/
