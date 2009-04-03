#! /bin/bash -eu
VERSION=$(git show --pretty=format:"%ai" HEAD | tr -cd [:digit:] | head -c 12)
DATA='GPL README armour/ configs/ models/ scripts/ sound/ ui/ vm/ -x  models/buildables/telenode/* models/weapons/ckit/*'
DATANAME=oc-$VERSION
TEXTURES='fonts/ gfx/ models/buildables/telenode/ models/weapons/ckit/ ui/assets emoticons/'
TEXTUREPAK='oc-textures-06'

if [ -e vm ]
then
  if [ -h vm ]
  then
    rm vm
  else
    echo "Error: file named 'vm' is not a symbolic link" >&2
    exit 1
  fi
fi

# find out what path make will build into
B=$(make -np 2> /dev/null | awk '$1 == "B" {print $3; exit 0}')
ln -s -- $B/base/vm/

echo "*** DIFF AND COMPILE  ***"

##nice -n15 make clean
#make clean
./generate-diff.sh

#if nice -n15 make
if make
then
#  mv current.patch oc-$VERSION.patch
else
#  rm current.patch
  exit 1
fi

echo "*** CLEARING OLD PK3S ***"
for PAK in "$DATANAME.pk3" "$TEXTUREPAK.pk3"
do if [ -e $PAK ]; then rm -vf $PAK; fi
done
echo "***  MAKING NEW PK3S  ***"
#zip -r $DATANAME.pk3 oc-$VERSION.patch $DATA
zip -r $DATANAME.pk3 $DATA
zip -r $TEXTUREPAK.pk3 $TEXTURES
