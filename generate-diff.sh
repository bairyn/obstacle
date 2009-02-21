#! /bin/bash
PATCHFILE=current.patch

UPSTREAM_VERSION=$(git svn find-rev origin/tremulous)
MGDEV_VERSION=$(git rev-parse master)

git diff origin/tremulous --src-prefix="tremulous-r$UPSTREAM_VERSION/" --dst-prefix="mgdev-$MGDEV_VERSION/" src/ Makefile make-macosx-ub.sh > $PATCHFILE
