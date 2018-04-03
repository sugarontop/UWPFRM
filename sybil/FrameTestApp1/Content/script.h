#pragma once

#define USE_EDGEMODE_JSRT
#include <jsrt.h>


#define MYEXPORT

struct js_context
{
	JsContextRef cxt;
	DWORD_PTR currentSourceContext;
	HANDLE runtime;
};


typedef JsValueRef (CALLBACK *export_function)(JsValueRef callee, bool isConstructCall, JsValueRef *arg, unsigned short argcnt, void *callbackState);

struct js_export_function
{
	LPCWSTR name;
	export_function func;
};

MYEXPORT js_context WINAPI js_appinit();
MYEXPORT int WINAPI js_create_context(struct js_context& h, js_export_function* functions=nullptr, int cnt_functions=0);
MYEXPORT void WINAPI js_app_exit(struct js_context& h) ;
MYEXPORT int  WINAPI js_run(struct js_context& h, LPCWSTR script);