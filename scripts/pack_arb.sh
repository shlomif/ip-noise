#!/bin/sh
source="$HOME/Docs/Univ/Project/from_cvs/C/arbitrator"
dest_base="IP-Noise-C-Arbitrator"
build_dir="$HOME/Arcs"

ver=`cat $source/ver.txt`
dest_dir="$dest_base-$ver"
dest_arc="$dest_dir.tar.gz"

cp -r "$source" "$build_dir/$dest_dir"
cd "$build_dir/$dest_dir"
# Clean some temporary files, editor backups, etc.
find . -name '*~' -print | xargs rm -f
find . -name '.*~' -print | xargs rm -f
find . -name '.*.swp' -print | xargs rm -f
find . -name 'CVS' -print | xargs rm -fr
find . -name '.cvsignore' -print | xargs rm -f
find . -name '*.o' -print | xargs rm -f
# Clean the executables
find . -type f -perm +0100 | xargs rm -f
cd ..
if test -e "$dest_arc" ; then
    echo "Error! An archive of the same version already exists"
    exit
fi

tar -czvf "$dest_arc" "$dest_dir"
rm -fr "$dest_dir"

