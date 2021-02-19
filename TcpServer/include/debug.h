#pragma once
#ifndef _DEBUG_H_
#define _DEBUG_H_
///*************************调试输出宏**************************************///
#if defined _DBG
#define _DEBUGGING_
#endif

#ifdef _DEBUGGING_
#include <stdio.h>
#endif

///@brief 一般放于函数末尾, 判断CPU是否执行函数成功
#ifdef _DEBUGGING_
#define COUT() printf("%s: %d rows %s succeed\n", __FILE__, __LINE__, __func__)
#else
#define COUT()
#endif

///@brief 打印变量的值
///@param name 变量的名称
///@param value 要查看的变量
#ifdef _DEBUGGING_
#define MYDBG(name, value) printf("%s:%x\n", name, value)
#else
#define MYDBG(name, value)
#endif

///@brief 功能等同与printf
///@brief SHOW(("num:%d", 0)); <=> printf("num:%d", 0);
#ifdef _DEBUGGING_
#define SHOW(args) printf args
#else
#define SHOW(args)
#endif

///@brief debug printf
///@brief SHOW(("num:%d", 0)); <=> printf("num:%d", 0);
#ifdef _DEBUGGING_
#define DPRT(...) printf(__VA_ARGS)
#else
#define DPRT(...)
#endif

#endif