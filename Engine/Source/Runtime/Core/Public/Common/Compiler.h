#pragma once

/*
* This is used to control the compiler (MSVC) compilation settings.
* Note that you should not use the tools here casually unless you know what you are doing.
*/

#define DISABLE_DLL_WARNINGS_PUSH __pragma(warning(push)) __pragma(warning(disable: 4251))
#define DISABLE_DLL_WARNINGS_POP __pragma(warning(pop))