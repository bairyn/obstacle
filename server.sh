#!/bin/bash
set -ueE -o pipefail
trap 'echo "An error occurred ($?) on line ${LINENO:-} (\$0 $0); aborting..." 1>&2' ERR

# Sample server start script.

# Dirs:
# ~/.tremulous-oc-server: read-only game server installed data.  Get it from the oc mod, GPP data, and 1.1 data.
# ~/.tremulous-oc-server-home: read-write game server app home dir.

# Set up server directories if they don't exist.
# Simple setup script.
if [[ ! -d ~/.tremulous-oc-server-home ]]; then
	install -d -m 0775 -- "${HOME}/.tremulous-oc-server-home"
fi
if [[ ! -d ~/.tremulous-oc-server ]]; then
	install -d -m 0775 -- "${HOME}/.tremulous-oc-server"
	install -d -m 0775 -- "${HOME}/.tremulous-oc-server/repo-1.1-builtin"
	install -d -m 0775 -- "${HOME}/.tremulous-oc-server/repo-1.1-gpp-builtin"
	install -d -m 0775 -- "${HOME}/.tremulous-oc-server/repo-oc"
	install -d -m 0775 -- "${HOME}/.tremulous-oc-server/base"

	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-oc/vms-oc.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-oc/oc-assets.pk3" ./)

	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-gpp-builtin/data-gpp1.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-gpp-builtin/maprotation.cfg" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-gpp-builtin/server.cfg" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-gpp-builtin/vms-gpp1.pk3" ./)

	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/data-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-arachnid2-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-atcs-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-karith-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-nexus6-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-niveus-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-transit-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-tremor-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/map-uncreation-1.1.0.pk3" ./)
	(cd -- "${HOME}/.tremulous-oc-server/base" && ln -ns -- "../repo-1.1-builtin/vms-1.1.0.pk3" ./)
fi
# Warn if the repos are empty.
for rel in \
	repo-oc/vms-oc.pk3 \
	repo-oc/oc-assets.pk3 \
 \
	repo-1.1-gpp-builtin/data-gpp1.pk3 \
	repo-1.1-gpp-builtin/maprotation.cfg \
	repo-1.1-gpp-builtin/server.cfg \
	repo-1.1-gpp-builtin/vms-gpp1.pk3 \
 \
	repo-1.1-builtin/data-1.1.0.pk3 \
	repo-1.1-builtin/map-arachnid2-1.1.0.pk3 \
	repo-1.1-builtin/map-atcs-1.1.0.pk3 \
	repo-1.1-builtin/map-karith-1.1.0.pk3 \
	repo-1.1-builtin/map-nexus6-1.1.0.pk3 \
	repo-1.1-builtin/map-niveus-1.1.0.pk3 \
	repo-1.1-builtin/map-transit-1.1.0.pk3 \
	repo-1.1-builtin/map-tremor-1.1.0.pk3 \
	repo-1.1-builtin/map-uncreation-1.1.0.pk3 \
	repo-1.1-builtin/vms-1.1.0.pk3 \
; do
	check_path="${HOME}/.tremulous-oc-server/${rel}"
	if [[ ! -e "${check_path}" ]]; then
		printf '%s\n' "ERROR: missing ro data package ‘${check_path}’; please copy it from oc mod, GPP, or 1.1 data." 1>&2
	fi
done

# Build if needed.
#make -j7 {BASE_,}"CFLAGS=$(pkg-config --cflags freetype2)"
# BASE_VERSION=1.1.0
make -j7 "CFLAGS=$(pkg-config --cflags freetype2)"

#false
# TODO: check time stamps.
(cd "./build/release-linux-x86_64/base" && zip -9r "${HOME}/.tremulous-oc-server/repo-oc/vms-oc.pk3" ./vm/*.qvm)
(cd "./assets" && zip -9r "${HOME}/.tremulous-oc-server/repo-oc/oc-assets.pk3" ./*)
./build/release-linux-x86_64/tremded.x86_64 +set fs_basepath "${HOME}/.tremulous-oc-server" +set fs_homepath "${HOME}/.tremulous-oc-server-home" +set net_port 30720 +exec oc-server.cfg +dedicated 1 "$@"
