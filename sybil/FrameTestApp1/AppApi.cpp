#include "pch.h"
#include "Entry.h"
#include "content/D2DUniversalControl.h"
#include "content/D2DTextbox.h"
#include "content/D2DDriftDialog.h"
#include "content/D2DWindowMessage.h"
#include "sybil.h"
#include "Content/script.h"
#include "Content/CJsValueRef.h"
#include "Content/InvokeHelper.h"

using namespace Windows::System::Threading;
using namespace V4;


extern D2DWindow* gparent;
extern D2DControls* gf1;
extern D2CoreTextBridge*  gimebridge;
extern std::map<IDispatch*,D2DControl*> gWindowMap;

static js_context app_script_context;

bool LoadScriptFile( LPCWSTR utf8filename, std::wstring& ret );


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
		rc.left = vx.ToFloat();
		CJsValueRef vy(arg[i++]);
		rc.top = vy.ToFloat();
		CJsValueRef vw(arg[i++]);
		sz.width = vw.ToFloat();
		CJsValueRef vh(arg[i++]);
		sz.height = vh.ToFloat();
		
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
		/*D2DTextbox* tx = new D2DTextbox(*gimebridge, ca ); 
		tx->Create(gparent, gf1, rc, VISIBLE, L"noname" );
		gWindowMap[disp.p] = tx;
		tx->SetTarget( disp.p );

		tx->SetText( text.c_str());*/
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


std::wstring fullpath_js_filename( std::wstring fnm )
{
	std::transform(fnm.begin(), fnm.end(), fnm.begin(), ::tolower);

	if ( fnm.length() < 3 || fnm.substr( fnm.length()-3, 3 ) != L".js" )
		fnm += L".js";

	std::wstring r; 
	r.resize( MAX_PATH );

	GetFullPathName( fnm.c_str(), MAX_PATH, &r[0], nullptr );
	return r;
}


JsValueRef CALLBACK Urequire(JsValueRef callee, bool isConstructCall, JsValueRef *arg, unsigned short argcnt, void *callbackState)
{
	int i = 1;
	CJsValueRef fnm(arg[i++]);

	std::wstring nm = fullpath_js_filename( fnm.ToString() );

	std::wstring src;
	if ( LoadScriptFile( nm.c_str(), src ) )			
	{
		JsValueRef ret1,exports_object;
		int r = js_run( app_script_context, src.c_str(), &ret1);

		// get exports object
		r = js_get_exports(app_script_context, &exports_object);
		
		return exports_object;
	}	
	return JS_INVALID_REFERENCE;
}



 bool JsOnEntryJavascript(js_context& ret)
{
	app_script_context = js_appinit();

	try 
	{
		js_export_function function[5];

		function[0].name = L"Log";
		function[0].func = ULog;
		function[1].name = L"CreateWindow";
		function[1].func = UCreateWindow;
		function[2].name = L"SetWindowText";
		function[2].func = USetWindowText;
		function[3].name = L"GetWindowText"; 
		function[3].func = UGetWindowText;
		function[4].name = L"require"; 
		function[4].func = Urequire;

		int r;

		js_create_context(app_script_context, function, 1 ); // sizeof(function)/sizeof(js_export_function) );
	
		std::wstring src;

		JsValueRef r1,r2;

		r = js_run( app_script_context, L"var exports={};", &r1 );

		if ( LoadScriptFile( L"Entry.js", src ) )		
		{
			auto er = js_run( app_script_context, src.c_str(), &r2);

			if ( er != 0 )
			{
				
				return false;
			}

			_ASSERT( er == 0 );

		}
	}
	catch( JsErrorCode  )
	{
		

	}

	ret = app_script_context;
	return true;
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

void JsOnAppEixt()
{
	js_app_exit( app_script_context );
}