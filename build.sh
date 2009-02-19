#!/bin/sh
if [ $# -lt 1 ]; then
	echo -ne "./build.sh <pakname>.pk3\n"
	exit 0
fi

if ! make; then
	exit
fi

if ! make debug; then
	exit
fi

if [ -d "build/release-linux-x86/base/configs" ]; then
	if ! rm -r build/release-linux-x86/base/configs; then
		exit
	fi
fi

if ! cp -R ./configs build/release-linux-x86/base/configs; then
	exit
fi

if [ -d "build/release-linux-x86/base/ui" ]; then
	if ! rm -r build/release-linux-x86/base/ui; then
		exit
	fi
fi

if ! cp -R ./ui build/release-linux-x86/base/ui; then
	exit
fi

if ! cp build/release-linux-x86/base/vm/game.qvm ~/.tremulousoc/oc/vm/; then
	exit
fi

if ! cp build/debug-linux-x86/base/gamex86.so ~/.tremulousoc/oc/; then
	exit
fi

if ! cd build/release-linux-x86/base/; then
	exit
fi

if [ -e ${1}.pk3 ]; then
	if ! rm ${1}.pk3; then
		exit
	fi
fi

if ! zip -9r ../../../${1}.pk3 vm/cgame.qvm vm/ui.qvm ui configs; then
	exit
fi

if ! cd ../../../; then
	exit
fi

if ! cp ${1}.pk3 /var/www/oc/; then
	exit
fi

if ! cp ${1}.pk3 ~/.tremulousoc/oc/; then
	exit
fi

# sometimes the binary copied can already be running
cp build/debug-linux-x86/tremded.x86 ~/.tremulousoc/oc/
