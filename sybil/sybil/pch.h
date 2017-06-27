#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <windows.h>
#include <comutil.h>
#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <strsafe.h>
#include <sstream>

#include "IBinary.h"
#include "comptr.h"

#include <MsXml6.h>

#define DEFAULTLOCALE L""


#undef _ASSERT
#define _ASSERT assert