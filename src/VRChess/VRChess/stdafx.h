// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#undef min
#undef max
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>
#include "d3dcompiler.h"
#include "d3d11.h"
#include "stdio.h"
#include <new>
#include <sstream>
#include <stack>
#include <map>
#include <unordered_map>
#include <limits>
#include <ctime>
#include <cassert>
#include <future>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
using namespace DirectX;
#include "OVR_CAPI_D3D.h"
using namespace std;