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

#define BSTR2(x) BSTRPtr(x).detach()

InnerApi innerapi;


int Class1::pin_hole(INT_PTR a)
{	
	DllBridge* aa = (DllBridge*)a;

	D2DControls* cs = aa->ctrls;
	
	auto df = (D2DControlfactory)aa->factory;

	innerapi.factory = df;
	
	auto mw = dynamic_cast<D2DMainWindow*>(cs->GetParentWindow());

	D2CoreTextBridge*  ime =  mw->GetImeBridge();

	Script* script = aa->script;

	FRectF rc(0,0,FSizeF(200,400));

	V4_XAPP1::D2DCells* tc = new V4_XAPP1::D2DCells();
	tc->Create(cs, rc, STAT::VISIBLE, L"メイリオ", 20, script);
	BSTRPtr data;

	if ( script->ExecBSTR( L"Initial();", &data))	
		tc->SetData( data );



	D2DControls* left = dynamic_cast<D2DControls*>(mw->FindControl(L"LeftControls" ));
	_ASSERT(left);

	rc.SetRect(10,100, FSizeF(50,25));
	V4_XAPP1::D2DButton* btn = new V4_XAPP1::D2DButton();
	btn->Create(left, rc, VISIBLE, L"TYPE1", NONAME );

	btn->OnClick_ = [](D2DButton* sender){
		sender->GetParentWindow()->SendMessage(WM_D2D_CELLS_CONTROLBAR_SHOW,0,nullptr);
	};


	
	rc.SetRect(8,200, FSizeF(80,25));
	
	D2DControl* lb = df(L"dropdownlistbox", left, nullptr, rc, L"lb");

	WParameterString wp;
	wp.str1 = BSTR2(L"1.0");
	wp.str2 = BSTR2(L"key2");
	lb->WndProc(mw, WM_D2D_LB_ADDITEM, (INT_PTR)&wp, nullptr);

	wp.str1 = BSTR2(L"0.8");
	wp.str2 = BSTR2(L"key20");
	lb->WndProc(mw, WM_D2D_LB_ADDITEM, (INT_PTR)&wp, nullptr);

	wp.str1 = BSTR2(L"1.1");
	wp.str2 = BSTR2(L"key21");
	lb->WndProc(mw, WM_D2D_LB_ADDITEM, (INT_PTR)&wp, nullptr);






	lb->WndProc( mw, WM_D2D_LB_SET_SELECT_IDX, (INT_PTR)0, nullptr);



	return 0;
}
