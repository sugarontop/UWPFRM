#include "pch.h"
#include "ComPTR.h"
#include "IBinary.h"
#include "httputil.h"
#include "internet.h"
#include "BSTRptr.h"

#pragma comment( lib, "msxml6")
using namespace sybil;

static int SequenceNumber = 0;

bool TryM( HRESULT hr, LPCWSTR msg )
{
	if ( hr != 0 )
		throw( msg );
	return true;
}

static bool FindString(BSTR ct, LPCWSTR str)
{
	std::wstring s = ct;
	return ((int)s.find(str) > -1 );
}


static std::vector<std::wstring> Split( LPCWSTR str, LPCWSTR split )
{
    std::vector<std::wstring> ar;
    int splen = wcslen(split);
    int len = wcslen(str);
    _ASSERT( 0 < splen && splen <= 2  );

    int si = 0;
    for( int i = 0; i <= len; i++ )
    {
        if ( str[i] == split[0] || (i == len && 0 < len) )
        {
            if (splen == 1 || (splen == 2 && (str[i+1] == split[1] || i == len)) )
            {
                std::wstring s( &str[si], i-si );
                ar.push_back(s);
                si = i + splen;
            }
        }       
    }
    return ar;
}
static std::map<std::wstring,std::wstring> ParseHeader( LPCWSTR headers_CRLF)
{
	
	std::map<std::wstring,std::wstring> headers;
	
	if ( headers_CRLF )
	{
		std::vector<std::wstring> rows = Split(headers_CRLF, L"\r\n");

		for(auto& row : rows )
		{
			auto keyval = Split(row.c_str(), L":");

			if ( keyval.size() == 2 )
				headers[keyval[0]] = keyval[1];
		}
	}
	return headers;
}


//bool GETInternet(LPCWSTR url, LPCWSTR headers, int* ret_retuslt, BSTR* ret_data)
//{	
//	IBinary bin;
//	BSTRPtr ct;	
//
//	bool bl = GETInternet(url, headers, ret_retuslt, &ct, &bin);
//
//	if ( bl && *ret_retuslt == 200 )
//	{			
//		IBinaryToBSTR( bin, ct, ret_data );		
//	}
//	return bl;
//}

//bool GETInternet(LPCWSTR url, LPCWSTR req_headers, int* ret_retuslt, BSTR* content, IBinary* ret_data)
//{	
//	bool bl = false;
//	
//	ComPTR<IXMLHTTPRequest2> request;
//	auto hr = CoCreateInstance(CLSID_XMLHTTP60, NULL, CLSCTX_ALL, IID_IXMLHTTPRequest2, (void**)&request);
//	if ( hr != S_OK )
//		return -1;
//
//	VARIANT v;
//	VariantInit(&v);
//	VARIANT v1 = v;
//	VARIANT v2 = v;
//	v2.vt = VT_BOOL;
//	v2.boolVal = VARIANT_FALSE;
//	BSTRPtr m(L"GET"), burl(url),ct(L"Content-Type"),ctlen(L"Content-Length"),rct,rctlen;
//
//	LPCWSTR pl = L"GETInternet";
//
//	//TryM(request.CoCreateInstance(CLSID_XMLHTTP60), pl); 
//
//	std::map<std::wstring, std::wstring> headers = ParseHeader(req_headers);
//
//
//	int cn = headers.size();
//	BSTRPtr* head = nullptr;
//	BSTRPtr* headval = nullptr;
//	if ( cn )
//	{
//		head = new BSTRPtr[cn];
//		headval = new BSTRPtr[cn];
//
//		int i = 0;
//		for(auto& it : headers )
//		{
//			head[i] = BSTRPtr(it.first.c_str());
//			headval[i] = BSTRPtr(it.second.c_str());
//			request->setRequestHeader( head[i],headval[i]);
//			i++;
//		}
//	}
//
//	TryM(request->open(	m,burl,v2,v1,v1), pl);
//	TryM(request->send(v1), pl);
//
//	// get status - 200 if succuss
//	long status;
//	TryM(request->get_status(&status), pl);
//
//	*ret_retuslt = (int)status;
//
//	delete [] head;
//	delete [] headval;
//	
//	if ( *ret_retuslt == 200 )
//	{	
//		// load image data (if url points to an image)
//		VARIANT responseVariant;
//		VariantInit(&responseVariant);
//		
//		TryM(request->get_responseStream(&responseVariant), pl);
//		IUnknown* pun = (IUnknown*)responseVariant.punkVal;
//
//		IStream* stream;
//		pun->QueryInterface(IID_IStream, (void**)&stream);
//
//		request->getResponseHeader(ct,&rct);
//		request->getResponseHeader(ctlen,&rctlen);
//
//		DWORD dw;
//		int len = _wtoi(rctlen);
//		BYTE* pv = new BYTE[len];
//		stream->Read(pv, len, &dw);
//
//		IBinary bin(IBinaryMk(pv,len));
//		*ret_data = bin;
//	
//		stream->Release();
//		pun->Release();		
//
//		*content = rct;
//		bl = true;
//	}
//	return bl;
//}




int GETInternetEx(LPCWSTR url, LPCWSTR request_headers, void* sender, InternetCallback ckf)
{	
	IXMLHTTPRequest2* req;
	auto hr = CoCreateInstance(CLSID_FreeThreadedXMLHTTP60, NULL, CLSCTX_ALL, IID_IXMLHTTPRequest2, (void**)&req);
	if ( hr != S_OK )
		return -1;

	MyRequest2Callback* ck = new MyRequest2Callback();
	
	ck->OnHeadersAvailable_ = [ck](IXMLHTTPRequest2* p, DWORD dwStatus, PCWSTR pwszStatus)
	{
		ck->stat_ = dwStatus;
	};
	ck->OnResponseReceived_ = [ck,ckf,sender](IXMLHTTPRequest2* req, ISequentialStream* pResponseStream)
	{
		DWORD rd;
		WCHAR cb[1024];
		ComPTR<IStream> is;
		auto hr = CreateStreamOnHGlobal(NULL, FALSE, &is);

		while (0 <= pResponseStream->Read(cb, 1024, &rd) && rd > 0)
		{
			DWORD rw;
			is->Write(cb, rd, &rw);
		}
		ck->server_message_ = ToIBinary(is);

		WCHAR* ct = {};
		WCHAR* headers = {};
		hr = req->GetResponseHeader(L"Content-Type", &ct);
		hr = req->GetAllResponseHeaders(&headers);

		
		ckf(sender, ck->stat_, ct, headers, ck->server_message_ );
	
		req->Release();	
		ck->Release();							
	};
	ck->OnError_ = [ck,ckf,sender](IXMLHTTPRequest2* req, HRESULT p1)
	{
		ck->stat_ = (int)p1;
		
		ckf(sender, ck->stat_, nullptr, nullptr, ck->server_message_ );

		req->Release();	
		ck->Release();
	};

	ComPTR<IXMLHTTPRequest2Callback> callback;
	hr = ck->QueryInterface(IID_IXMLHTTPRequest2Callback, (void**)&callback);
	hr = req->Open(L"GET", url, callback, nullptr, nullptr, nullptr, nullptr);

	if ( request_headers )
	{
		std::map<std::wstring,std::wstring> headers = ParseHeader(request_headers);

		for (auto& it : headers)
			req->SetRequestHeader(it.first.c_str(), it.second.c_str());
	}

	hr = req->Send(nullptr, 0);
	
	return ++SequenceNumber;
}

int POSTInternetEx(LPCWSTR url, LPCWSTR req_headers_CRLF, byte* body, ULONG bodylen,  void* sender, InternetCallback ckf)
{
	IXMLHTTPRequest2* req;
	auto hr = CoCreateInstance(CLSID_FreeThreadedXMLHTTP60, NULL, CLSCTX_ALL, IID_IXMLHTTPRequest2, (void**)&req);

	MyRequest2Callback* ck = new MyRequest2Callback();

	ck->OnHeadersAvailable_ = [ck](IXMLHTTPRequest2* p, DWORD dwStatus, PCWSTR pwszStatus)
	{
		ck->stat_ = dwStatus;
		
	};
	ck->OnResponseReceived_ = [ck, sender,ckf](IXMLHTTPRequest2* req, ISequentialStream* pResponseStream)
	{
		DWORD rd;
		WCHAR cb[1024];
		ComPTR<IStream> is;
		auto hr = CreateStreamOnHGlobal(NULL, FALSE, &is);

		while (0 <= pResponseStream->Read(cb, 1024, &rd) && rd > 0)
		{
			DWORD rw;
			is->Write(cb, rd, &rw);
		}
		ck->server_message_ = ToIBinary(is);

		
		WCHAR* ct = {};
		WCHAR* headers = {};
		hr = req->GetResponseHeader(L"Content-Type", &ct);
		hr = req->GetAllResponseHeaders(&headers);

		ckf(sender, ck->stat_, ct, headers, ck->server_message_ );
				
		req->Release();
		ck->Release();
	};

	ck->OnError_ = [ck,ckf,sender](IXMLHTTPRequest2* req, HRESULT p1)
	{
		ck->stat_ = (int)p1;
		
		
		ckf(sender, ck->stat_, nullptr, nullptr, ck->server_message_ );

		req->Release();	
		ck->Release();
	};

	ComPTR<IXMLHTTPRequest2Callback> callback;
	ck->QueryInterface(IID_IXMLHTTPRequest2Callback, (void**)&callback);

	req->Open(L"POST", url, callback, nullptr, nullptr, nullptr, nullptr);

	if ( req_headers_CRLF )
	{
		std::map<std::wstring,std::wstring> headers = ParseHeader(req_headers_CRLF);

		for (auto& it : headers)
			req->SetRequestHeader(it.first.c_str(), it.second.c_str());
	}

	ComPTR<IStream> bodystream;
			
	auto r = CreateStreamOnHGlobal(NULL, FALSE, &bodystream);
	ULONG wrbyte;
	bodystream->Write(body, bodylen, &wrbyte );

	req->Send(bodystream, wrbyte);
	
	return ++SequenceNumber;
}
