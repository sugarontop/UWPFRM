#pragma once
#include <msxml6.h>
#include "IBinary.h"


class MyRequest2Callback : public IXMLHTTPRequest2Callback
{
	LONG refcnt_;
public:
	MyRequest2Callback():refcnt_(1),callback_(nullptr),resdata_(nullptr){}

	//IUnknown ----------------------------------------------------------------------------------
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&refcnt_);
	};
	STDMETHODIMP_(ULONG) Release()
	{
		InterlockedDecrement(&refcnt_);

		if (refcnt_ > 0)
			return refcnt_;

		delete this;
		return refcnt_;
	}
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvOut)
	{
		if (*ppvOut)
			*ppvOut = NULL;

		if (IsEqualIID(riid, IID_IXMLHTTPRequest2Callback))
			*ppvOut = (IXMLHTTPRequest2Callback*)this;
		else if (IsEqualIID(riid, IID_IUnknown))
			*ppvOut = this;
		else
			return E_NOINTERFACE;

		AddRef();
		return S_OK;
	}

	//IXMLHTTPRequest2Callback ----------------------------------------------------------------------------------
	// Called when the HTTP request is being redirected to a new URL.
	STDMETHODIMP STDMETHODCALLTYPE OnRedirect(
		/* [in] */ __RPC__in_opt IXMLHTTPRequest2 *pXHR,
		/* [string][in] */ __RPC__in_string const WCHAR *pwszRedirectUrl)
	{
		if (OnRedirect_)
			OnRedirect_(pXHR, pwszRedirectUrl);
		return S_OK;
	}

	// Called when HTTP headers have been received and processed.
	STDMETHODIMP STDMETHODCALLTYPE OnHeadersAvailable(IXMLHTTPRequest2* p, DWORD dwStatus, PCWSTR pwszStatus)
	{
		if (OnHeadersAvailable_)
			OnHeadersAvailable_(p, dwStatus, pwszStatus);
		return S_OK;
	}
	STDMETHODIMP STDMETHODCALLTYPE OnResponseReceived(IXMLHTTPRequest2* p, ISequentialStream* pResponseStream)
	{

		if (OnResponseReceived_)
			OnResponseReceived_(p, pResponseStream);
		return S_OK;
	}
	// Called when a portion of the entity body has been received.
	STDMETHODIMP STDMETHODCALLTYPE OnDataAvailable(IXMLHTTPRequest2* p, ISequentialStream* p1)
	{
		if (OnDataAvailable_)
			OnDataAvailable_(p, p1);
		return S_OK;
	}
	STDMETHODIMP STDMETHODCALLTYPE OnError(IXMLHTTPRequest2* p, HRESULT p1)
	{
		if (OnError_)
			OnError_(p, p1);

		return S_OK;
	}

	std::function<void(IXMLHTTPRequest2*, ISequentialStream* pResponseStream)> OnResponseReceived_;
	std::function<void(IXMLHTTPRequest2*, PCWSTR)> OnRedirect_;
	std::function<void(IXMLHTTPRequest2*, DWORD, PCWSTR)> OnHeadersAvailable_;
	std::function<void(IXMLHTTPRequest2*, ISequentialStream*)> OnDataAvailable_;
	std::function<void(IXMLHTTPRequest2*, HRESULT hrError)> OnError_;

	int stat_;
	IBinary server_message_;
	void* callback_;
	void* resdata_;
};



// NON BLOCKING
typedef void  (*InternetCallback)(void* sender,int result, LPCWSTR content, LPCWSTR res_heders_CRLF, IBinary& response);

int GETInternetEx(LPCWSTR url, LPCWSTR req_headers_CRLF, void* sender, InternetCallback ckf);
int POSTInternetEx(LPCWSTR url, LPCWSTR req_headers_CRLF, byte* req_body, ULONG req_bodylen,  void* sender, InternetCallback ckf);