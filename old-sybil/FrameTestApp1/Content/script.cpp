
#include "pch.h"
#include "script.h"

static void FAIL_IS_THROW(JsErrorCode er )
{
	if ( er == JsNoError ) return; 

	throw er;

}

JsErrorCode CreateHostContext(JsRuntimeHandle runtime, int argc, wchar_t *argv [], int argumentsStart, JsContextRef *context);
JsErrorCode DefineHostCallback(JsValueRef globalObject, const wchar_t *callbackName, JsNativeFunction callback, void *callbackState);


MYEXPORT js_context WINAPI js_appinit()
{
	HANDLE runtime;
	FAIL_IS_THROW(JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime));

	js_context r;
	r.runtime = runtime;
	r.currentSourceContext = 0;
	r.cxt = 0;

	return r;	
}
MYEXPORT int WINAPI js_create_context(struct js_context& h, js_export_function* functions, int cnt_functions)
{
	HANDLE runtime = h.runtime;

	JsContextRef context;
	int argc = 1;
	WCHAR* argv[1]={0};

	FAIL_IS_THROW(CreateHostContext(runtime, argc,argv,1,&context));
	
	FAIL_IS_THROW(JsSetCurrentContext(context));

	h.cxt = context;

	JsValueRef globalObject;
	FAIL_IS_THROW(JsGetGlobalObject(&globalObject));

	for( int i = 0; i < cnt_functions; i++ )
		FAIL_IS_THROW(DefineHostCallback(globalObject, functions[i].name, functions[i].func, nullptr));

	return 0;

}
MYEXPORT void WINAPI js_app_exit(struct js_context& h) 
{
	if ( h.runtime )
	{
		JsSetCurrentContext(JS_INVALID_REFERENCE); 
	
		JsDisposeRuntime(h.runtime);
		h.runtime = nullptr;
	}
}


MYEXPORT JsErrorCode  WINAPI js_run(struct js_context& h, LPCWSTR script, JsValueRef* result )
{	
	FAIL_IS_THROW(JsSetCurrentContext( h.cxt )); 	
	JsErrorCode errorCode = JsRunScript(script, h.currentSourceContext++, L"", result);
	return errorCode;
}

MYEXPORT int  WINAPI js_get_exports(struct js_context& h, JsValueRef* result)
{
	JsPropertyIdRef x;
	FAIL_IS_THROW(JsGetPropertyIdFromName(L"exports", &x));

	JsValueRef globalObject;
	FAIL_IS_THROW(JsGetGlobalObject(&globalObject));
	FAIL_IS_THROW(JsGetProperty(globalObject, x, result));

	return 0;
}

JsErrorCode CreateHostContext(JsRuntimeHandle runtime, int argc, wchar_t *argv [], int argumentsStart, JsContextRef *context)
{	
	FAIL_IS_THROW(JsCreateContext(runtime, context));	
	FAIL_IS_THROW(JsSetCurrentContext(*context));	
	JsValueRef hostObject;
	FAIL_IS_THROW(JsCreateObject(&hostObject));
	JsValueRef globalObject;
	FAIL_IS_THROW(JsGetGlobalObject(&globalObject));
	
	JsPropertyIdRef hostPropertyId;
	FAIL_IS_THROW(JsGetPropertyIdFromName(L"host", &hostPropertyId));

	FAIL_IS_THROW(JsSetProperty(globalObject, hostPropertyId, hostObject, true));

	JsValueRef arguments;
	FAIL_IS_THROW(JsCreateArray(argc - argumentsStart, &arguments));

	for (int index = argumentsStart; index < argc; index++)
	{
		// Create the argument value.
		JsValueRef argument;
		FAIL_IS_THROW(JsPointerToString(argv[index], wcslen(argv[index]), &argument));

		// Create the index.
		JsValueRef indexValue;
		FAIL_IS_THROW(JsIntToNumber(index - argumentsStart, &indexValue));

		// Set the value.
		FAIL_IS_THROW(JsSetIndexedProperty(arguments, indexValue, argument));
	}

	JsPropertyIdRef argumentsPropertyId;
	FAIL_IS_THROW(JsGetPropertyIdFromName(L"arguments", &argumentsPropertyId));

	FAIL_IS_THROW(JsSetProperty(hostObject, argumentsPropertyId, arguments, true));


	FAIL_IS_THROW(JsSetCurrentContext(JS_INVALID_REFERENCE));

	return JsNoError;
}
JsErrorCode DefineHostCallback(JsValueRef globalObject, const wchar_t *callbackName, JsNativeFunction callback, void *callbackState)
{
	JsPropertyIdRef propertyId;
	FAIL_IS_THROW(JsGetPropertyIdFromName(callbackName, &propertyId));

	JsValueRef function;
	FAIL_IS_THROW(JsCreateFunction(callback, callbackState, &function));

	FAIL_IS_THROW(JsSetProperty(globalObject, propertyId, function, true));

	return JsNoError;
}


