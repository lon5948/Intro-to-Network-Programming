#!/bin/bash

mkdir build

make

sudo mount -t nfs4 -o nfsvers=4.1,rsize=1048576,wsize=1048576,hard,timeo=600,retrans=2,noresvport fs-0e79ecb5f6167471d.efs.us-east-1.amazonaws.com:/ efs

sudo su


