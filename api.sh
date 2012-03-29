#!/bin/bash

exec > scim/api.h
sed -n -f api.sed srvc_{cell,crew,host,note,port,root,site,task,team,term,tool}.c
