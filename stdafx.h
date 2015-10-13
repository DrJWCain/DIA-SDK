// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <iostream>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "dia2.h"
#include <atlbase.h>

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <list>
#include <set>
#include <fstream>
#include <algorithm>
#include <iomanip>

inline void checkResult(HRESULT hr)
{
  if(FAILED(hr))
    throw hr;
}
