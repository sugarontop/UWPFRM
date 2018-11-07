#include "pch.h"
#include "Class1.h"

#include "D2DUniversalControlBase.h"
#include "D2DTextbox.h"
#include "D2DInputControl.h"
#include "Entry.h"
#include "..\sybil\sybil.h"

#include "BSTRptr.h"
#include "HiggsJson.h"

#pragma comment(lib,"sybil")

using namespace XApp1;
using namespace Platform;
using namespace V4;
using namespace V4_XAPP1;
using namespace sybil;


int Class1::pin_hole(INT_PTR a)
{	
	DllBridge* aa = (DllBridge*)a;

	D2DControls* cs = aa->ctrls;

	D2CoreTextBridge*  ime =  dynamic_cast<D2DMainWindow*>(cs->GetParentWindow())->GetImeBridge();

	Script* script = aa->script;

	FRectF rc(0,0,200,400);

	D2DCells* tc = new D2DCells();
	tc->Create(cs, rc, STAT::VISIBLE, L"メイリオ", 20, script);
	BSTRPtr data;

	if ( script->ExecBSTR( L"Initial();", &data))	
		tc->SetData( data );


	return 0;
}
