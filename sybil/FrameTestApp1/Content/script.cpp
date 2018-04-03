
#include "pch.h"
#include "script.h"

void IfFailRet(JsErrorCode er )
{
	auto k = er;

	_ASSERT(k == JsNoError);

}

JsErrorCode CreateHostContext(JsRuntimeHandle runtime, int argc, wchar_t *argv [], int argumentsStart, JsContextRef *context);
JsErrorCode DefineHostCallback(JsValueRef globalObject, const wchar_t *callbackName, JsNativeFunction callback, void *callbackState);
JsValueRef CALLBACK echo(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, _In_opt_ void *callbackState);

MYEXPORT js_context WINAPI js_appinit()
{
	HANDLE runtime;
	JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);

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

	CreateHostContext(runtime, argc,argv,1,&context);// argc, argv, arguments.argumentsStart, &context); //, L"failed to create execution context.");
	
	JsSetCurrentContext(context);

	h.cxt = context;

	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);

	for( int i = 0; i < cnt_functions; i++ )
		DefineHostCallback(globalObject, functions[i].name, functions[i].func, nullptr);

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


MYEXPORT int  WINAPI js_run(struct js_context& h, LPCWSTR script )
{
	
	JsSetCurrentContext( h.cxt ); 

	JsValueRef result;
	JsErrorCode errorCode = JsRunScript(script, h.currentSourceContext++, L"", &result);

	return errorCode;
}

JsErrorCode CreateHostContext(JsRuntimeHandle runtime, int argc, wchar_t *argv [], int argumentsStart, JsContextRef *context)
{
	
	JsCreateContext(runtime, context);
	
	JsSetCurrentContext(*context);

	
	JsValueRef hostObject;
	JsCreateObject(&hostObject);

	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);

	
	JsPropertyIdRef hostPropertyId;
	JsGetPropertyIdFromName(L"host", &hostPropertyId);

	JsSetProperty(globalObject, hostPropertyId, hostObject, true);


	

	IfFailRet(DefineHostCallback(globalObject, L"echo", echo, nullptr));


//	IfFailRet(DefineHostCallback(hostObject, L"api", apiFunc, nullptr));


	JsValueRef arguments;
	JsCreateArray(argc - argumentsStart, &arguments);

	for (int index = argumentsStart; index < argc; index++)
	{
		//
		// Create the argument value.
		//

		JsValueRef argument;
		IfFailRet(JsPointerToString(argv[index], wcslen(argv[index]), &argument));

		//
		// Create the index.
		//

		JsValueRef indexValue;
		IfFailRet(JsIntToNumber(index - argumentsStart, &indexValue));

		//
		// Set the value.
		//

		IfFailRet(JsSetIndexedProperty(arguments, indexValue, argument));
	}

	JsPropertyIdRef argumentsPropertyId;
	JsGetPropertyIdFromName(L"arguments", &argumentsPropertyId);

	JsSetProperty(hostObject, argumentsPropertyId, arguments, true);


	JsSetCurrentContext(JS_INVALID_REFERENCE);

	return JsNoError;
}
JsErrorCode DefineHostCallback(JsValueRef globalObject, const wchar_t *callbackName, JsNativeFunction callback, void *callbackState)
{
	JsPropertyIdRef propertyId;
	IfFailRet(JsGetPropertyIdFromName(callbackName, &propertyId));

	JsValueRef function;
	IfFailRet(JsCreateFunction(callback, callbackState, &function));

	IfFailRet(JsSetProperty(globalObject, propertyId, function, true));

	return JsNoError;
}
JsValueRef CALLBACK echo(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, _In_opt_ void *callbackState)
{
	for (unsigned int index = 1; index < argumentCount; index++)
	{
		/*CJsValueRef v(arguments[index]);
		auto s = v.ToString();

		std::wcout << s.c_str() << L"\n";*/


		//TRACE( L"echo %s\n", s.c_str() );

	}
	return JS_INVALID_REFERENCE;
}

