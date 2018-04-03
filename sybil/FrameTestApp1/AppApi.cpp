#include "pch.h"
#include "Entry.h"
#include "content/D2DUniversalControl.h"
#include "content/D2DDriftDialog.h"
#include "content/D2DWindowMessage.h"
#include "sybil.h"
#include "SampleTest.h"
#include "Content/script.h"
#include "Content/CJsValueRef.h"
#include "Content/InvokeHelper.h"

using namespace Windows::System::Threading;
using namespace V4;


extern D2DWindow* gparent;
extern D2DChildFrame* gf1;
extern D2CoreTextBridge*  gimebridge;
extern std::map<IDispatch*,D2DControl*> gWindowMap;


 D2DControl* GetTargetControl( JsValueRef& r )
 {
	CJsValueRef obj(r);
	ComPTR<IDispatch> disp;
	obj.ToIDispatch(&disp);
	return gWindowMap[disp.p];
 }

  JsValueRef CALLBACK UCreateWindow(JsValueRef callee, bool isConstructCall, JsValueRef *arg, unsigned short argcnt, void *callbackState)
 {
	std::wstring text,typ;
	FRectF rc;
	FSizeF sz;

	ComPTR<IDispatch> disp;

	{
		int i = 1;
		
		CJsValueRef obj(arg[i++]);
		obj.ToIDispatch(&disp);

		CJsValueRef t(arg[i++]);
		typ = t.ToString();

		CJsValueRef v(arg[i++]);
		text = v.ToString();

		CJsValueRef vx(arg[i++]);
		rc.left = vx.ToInt();
		CJsValueRef vy(arg[i++]);
		rc.top = vy.ToInt();
		CJsValueRef vw(arg[i++]);
		sz.width = vw.ToInt();
		CJsValueRef vh(arg[i++]);
		sz.height = vh.ToInt();
		
		rc.SetSize(sz);
	}

	if ( typ == L"button" )
	{
		D2DButton* btn = new D2DButton();
		btn->Create( gparent, gf1, rc, VISIBLE, text.c_str(), L"noname" );
		gWindowMap[disp.p] = btn;

		btn->SetTarget( disp.p );

		btn->OnClick_ = [](D2DButton* b)
		{
			variant v;	
			InvokeMethod0(b->GetTarget(), L"OnClick", v);			
		};
	}
	else if ( typ == L"textbox" )
	{		
		D2DTextbox* tx = new D2DTextbox(*gimebridge);
		tx->Create(gparent, gf1, rc, VISIBLE, L"noname" );
		gWindowMap[disp.p] = tx;
		tx->SetTarget( disp.p );

		tx->SetText( text.c_str());

		
		//variant vlen;
		//auto hr = InvokeGetProperty( tx->GetTarget(), L"TextLength", vlen );
	}

	return JS_INVALID_REFERENCE;
 }
JsValueRef CALLBACK UGetWindowText(JsValueRef callee, bool isConstructCall, JsValueRef *arg, unsigned short argcnt, void *callbackState)
{
	int i = 1;
	if ( 1 < argcnt )
	{
		D2DControl* p = (GetTargetControl( arg[i++]));

		auto s = p->GetText();

		CJsValueRef r(s.c_str());
		return (JsValueRef)r; 
	}
	return JS_INVALID_REFERENCE;
}


JsValueRef CALLBACK USetWindowText(JsValueRef callee, bool isConstructCall, JsValueRef *arg, unsigned short argcnt, void *callbackState)
{
	int i = 1;

	if ( 2 < argcnt )
	{
		D2DControl* p = (GetTargetControl( arg[i++]));
		
		CJsValueRef text(arg[i++]);
		
		auto s = text.ToString();

		p->SetText( s.c_str() );		
	}
	return JS_INVALID_REFERENCE;
}
 JsValueRef CALLBACK ULog(JsValueRef callee, bool isConstructCall, JsValueRef *arg, unsigned short argcnt, void *callbackState)
{
	int i = 1;
	CJsValueRef t(arg[i++]);
	auto typ = t.ToString();
	return JS_INVALID_REFERENCE;
}
 
extern js_context app_script_context;
bool LoadScriptFile( LPCWSTR utf8filename, std::wstring& ret );

void OnEntryJavascript()
{
	app_script_context = js_appinit();

	js_export_function function[4];

	function[0].name = L"CreateWindow";
	function[0].func = UCreateWindow;
	function[1].name = L"Log";
	function[1].func = ULog;
	function[2].name = L"SetWindowText";
	function[2].func = USetWindowText;
	function[3].name = L"GetWindowText"; 
	function[3].func = UGetWindowText;

	int r;

	js_create_context(app_script_context, function, sizeof(function)/sizeof(js_export_function) );
	
	std::wstring src;

	if ( LoadScriptFile( L"init.js", src ) )			
		r = js_run( app_script_context, src.c_str());
}


bool LoadScriptFile( LPCWSTR utf8filename, std::wstring& ret )
{
	CREATEFILE2_EXTENDED_PARAMETERS cfprm = {0};
	cfprm.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	cfprm.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
	cfprm.dwFileFlags = FILE_FLAG_NO_BUFFERING;
	cfprm.dwSecurityQosFlags = SECURITY_ANONYMOUS;
	cfprm.lpSecurityAttributes = NULL;
	cfprm.hTemplateFile = NULL;

	HANDLE h = CreateFile2( utf8filename, GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING,&cfprm);
	DWORD error = GetLastError();

	std::stringstream sm;
	std::wstring src;

	if ( h != INVALID_HANDLE_VALUE )
	{
		char cb[512];
		DWORD dw;

		while ( ReadFile( h, cb, 512,&dw,nullptr) && dw )
		{
			sm.write( cb, dw );
		}

		// binary->utf8
		auto s = sm.str();
		int len = MultiByteToWideChar(CP_UTF8,0, s.c_str(), s.length(), NULL,NULL );
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), &ret[0], len );
		
		::CloseHandle(h);
		return true;
	}
	return false;
}