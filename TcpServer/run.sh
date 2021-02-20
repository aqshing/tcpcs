#!/bin/bash
###################################################
# Filename: run.sh
# Author: shing
# Email: www.asm.best
# Created: 2021年02月19日 星期五 18时14分21秒
###################################################
##set -xe

function do_help()
{
    echo  "Usage:"
    echo  "    run.sh  [compile] [debug|release]"

    return  0
}

function do_compile()
{
    cd $(dirname $0)/build

    #   确定与构建模式相关的参数
    if   [[ "$2" == "debug" ]] || [[ "$2" == "" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Debug ..
    elif [[ "$2" == "release" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Release ..
    else
        echo "Error: Unsupported compile mode '$2'"
        return 1
    fi

    make clean
    make -j4

    ../bin/server

    return 0
}

function main()
{
	local action="$1"

    if [[ "${action}" == "help" ]] || [[ "${action}" == "--help" ]] || [[ "${action}" == "-h" ]]; then
        do_help     "$@"
        return      "$?"
    fi

    if [[ "${action}" == "debug" ]] || [[ "${action}" == "release" ]] || [[ "${action}" == "" ]]; then
        do_compile  "compile" "$@"
        return      "$?"
    fi

    if [[ "${action}" == "compile" ]]; then
        do_compile  "$@"
        return      "$?"
    fi

    echo  "Unsupported action '${action}', type 'help' for help"
    return  $?
}

main  "$@"
exit  "$?"
