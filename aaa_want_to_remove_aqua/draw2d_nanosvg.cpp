﻿#include "platform/platform/config.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WINDOWS
#pragma warning(disable:4101)
#endif

#define NANOSVG_IMPLEMENTATION  // Expands implementation
#include "nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvgrast.h"

