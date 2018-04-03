#pragma once

#include "content/D2DUniversalControl.h"
#include "content/D2DContext.h"
#include "content/D2DWindowMessage.h"

typedef _variant_t variant;


void OnEntry(V4::D2DWindow* parent, V4::FSizeF iniSz, V4::D2CoreTextBridge* imebridge);
void OnExit();