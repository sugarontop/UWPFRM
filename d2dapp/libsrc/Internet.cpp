#include "pch.h"
#include "ComPTR.h"
#include "IBinary.h"
#include "httputil.h"
#include "internet.h"
#include "BSTRptr.h"

#pragma comment( lib, "msxml6")


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
		ck->server_message_ = V4::ToIBinary(is);

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
		ck->server_message_ = V4::ToIBinary(is);

		
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
