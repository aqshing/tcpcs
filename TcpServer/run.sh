#!/bin/bash
###################################################
# Filename: run.sh
# Author: shing
# Email: www.asm.best
# Created: 2021年02月19日 星期五 18时14分21秒
###################################################
set -xeuo pipefail

cd ./build

cmake -DCMAKE_BUILD_TYPE=Debug ..

make -j4

../bin/server
