#include "pch.h"
#include "ComPTR.h"
#include "IBinary.h"
#include "httputil.h"
#include "internet.h"

//#include "bitflyv1.h"

#pragma comment( lib, "msxml6")
using namespace sybil;

void GETInternetEx(LPCWSTR url, std::map<std::wstring, std::wstring>& headers, int* ret_retuslt, BSTR* ret_json, IXMLHTTPRequest2Callback** ret)
{	
	IXMLHTTPRequest2* req;
	auto hr = CoCreateInstance(CLSID_FreeThreadedXMLHTTP60, NULL, CLSCTX_ALL, IID_IXMLHTTPRequest2, (void**)&req);
	if ( hr != S_OK )
	{
		*ret_retuslt = -11;
		return;
	}




	MyRequest2Callback* ck = new MyRequest2Callback();

	*ret_retuslt = 0;
	ck->OnHeadersAvailable_ = [ck, ret_retuslt](IXMLHTTPRequest2* p, DWORD dwStatus, PCWSTR pwszStatus)
	{

		ck->stat_ = dwStatus;

		//WCHAR* ct;
		//p->GetResponseHeader( L"Content-Type", &ct );


		*ret_retuslt = -2;
	};
	ck->OnResponseReceived_ = [ck,ret_json, ret_retuslt](IXMLHTTPRequest2* p, ISequentialStream* pResponseStream)
	{
		//DebugWrite( L"OnResponseReceived_" );

			DWORD rd;
			WCHAR cb[1024];
			ComPTR<IStream> is;
			auto r = CreateStreamOnHGlobal(NULL, FALSE, &is);

			while (0 <= pResponseStream->Read(cb, 1024, &rd) && rd > 0)
			{
				DWORD rw;
				is->Write(cb, rd, &rw);
			}
			ck->server_message_ = ToIBinary(is);

			BSTR bs;
			Utf8ToBSTR(ck->server_message_, &bs);

 			*ret_json = bs;
			*ret_retuslt = ck->stat_;

			p->Release();		
	};

	ComPTR<IXMLHTTPRequest2Callback> callback;
	ck->QueryInterface(IID_IXMLHTTPRequest2Callback, (void**)&callback);

	req->Open(L"GET", url, callback, nullptr, nullptr, nullptr, nullptr);

	for (auto& it : headers)
		req->SetRequestHeader(it.first.c_str(), it.second.c_str());

	hr = req->Send(nullptr, 0);

	*ret = callback;
	(*ret)->AddRef();
	
	if ( hr == S_OK )
		*ret_retuslt = -1;
	else
		*ret_retuslt = -10;
}

void POSTInternetEx(LPCWSTR url, std::map<std::wstring, std::wstring>& headers,IBinary& body, int* ret_retuslt, BSTR* ret_json, IXMLHTTPRequest2Callback** ret)
{
	IXMLHTTPRequest2* req;
	auto hr = CoCreateInstance(CLSID_FreeThreadedXMLHTTP60, NULL, CLSCTX_ALL, IID_IXMLHTTPRequest2, (void**)&req);

	MyRequest2Callback* ck = new MyRequest2Callback();

	*ret_retuslt = 0;

	ck->OnHeadersAvailable_ = [ck, ret_retuslt](IXMLHTTPRequest2* p, DWORD dwStatus, PCWSTR pwszStatus)
	{
		ck->stat_ = dwStatus;

		//WCHAR* ct;
		//p->GetResponseHeader( L"Content-Type", &ct );
		*ret_retuslt = -2;
	};
	ck->OnResponseReceived_ = [ck, ret_json, ret_retuslt](IXMLHTTPRequest2* p, ISequentialStream* pResponseStream)
	{
		//DebugWrite(L"OnResponseReceived_");

		DWORD rd;
		WCHAR cb[1024];
		ComPTR<IStream> is;
		auto r = CreateStreamOnHGlobal(NULL, FALSE, &is);

		while (0 <= pResponseStream->Read(cb, 1024, &rd) && rd > 0)
		{
			DWORD rw;
			is->Write(cb, rd, &rw);
		}
		ck->server_message_ = ToIBinary(is);

		BSTR bs;
		Utf8ToBSTR(ck->server_message_, &bs);

		*ret_json = bs;
		*ret_retuslt = ck->stat_;

		p->Release();
	};

	ComPTR<IXMLHTTPRequest2Callback> callback;
	ck->QueryInterface(IID_IXMLHTTPRequest2Callback, (void**)&callback);

	req->Open(L"POST", url, callback, nullptr, nullptr, nullptr, nullptr);

	for (auto& it : headers)
		req->SetRequestHeader(it.first.c_str(), it.second.c_str());

	ComPTR<IStream> bodystream;
			
	auto r = CreateStreamOnHGlobal(NULL, FALSE, &bodystream);
	ULONG ww;
	bodystream->Write(IBinaryPtr(body), IBinaryLen(body), &ww );

	req->Send(bodystream, ww);
	

	*ret = callback;
	(*ret)->AddRef();

	*ret_retuslt = -1;
}