#!/bin/bash

declare -r srcdir=$1
exec > $srcdir/scim/api.h
sed -n -f $srcdir/api.sed $srcdir/{scim_{cell,cpio,crew,host,loop,lzma,note,port,root,site,task,team,term,tool},btrfs}.c
