#pragma once

#include "panic.h"

#define _assert(lineno, filename, stmt) \
{ \
if (!(stmt)) \
	_panic(lineno, filename, "\e[31massert failed\e[m: %s", #stmt); \
}

#define assert(stmt) _assert(__LINE__, __FILE__, stmt)
