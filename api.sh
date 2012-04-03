#!/bin/bash

declare -r srcdir=$1
exec > $srcdir/scim/api.h
sed -n -f $srcdir/api.sed $srcdir/scim_{cell,crew,host,note,port,root,site,task,team,term,tool}.c
