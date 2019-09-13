#include "pch.h"

#include "sybil.h"
#include "content/d2dmisc.h"
#include "internet.h"
#include "httputil.h"


using namespace V4;
namespace sybil {


DLLEXPORT bool WINAPI CreateTextFromat(IDWriteFactory* wfac, LPCWSTR fontname, float height, int weight, IDWriteTextFormat** tf  )
{	
	auto hr =
	wfac->CreateTextFormat(
		fontname,
		nullptr,
		(DWRITE_FONT_WEIGHT)weight, // 400:normal
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		height,
		DEFAULTLOCALE,
		tf
	);

	return ( S_OK == hr );
}

DLLEXPORT bool WINAPI CreateSingleTextLayout(IDWriteFactory* wfac, LPCWSTR str, int length, IDWriteTextFormat* tf, IDWriteTextLayout** tl  )
{
	return (S_OK == wfac->CreateTextLayout( str,length, tf, 32000,32000, tl ));
}

DLLEXPORT void WINAPI DrawTextLayoutCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, IDWriteTextLayout* tl, ID2D1Brush* br  )
{	
	DWRITE_TEXT_METRICS tm;
	tl->GetMetrics(&tm);

	FPointF pt;
	
	pt.x = ((rc.right - rc.left) - tm.width)/2.0f;
	pt.y = ((rc.bottom - rc.top) - tm.height)/2.0f;
	
	p->DrawTextLayout( pt, tl, br );
}



DLLEXPORT void WINAPI DrawTextCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* br  )
{
	auto old = tf->GetTextAlignment();
	tf->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER );	
	p->DrawText( str, length, tf, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	tf->SetTextAlignment( old );
}

DLLEXPORT HANDLE WINAPI DrawDriftRect( HANDLE cxt, D2D1_RECT_F* ret, DWORD ticknow, const D2D1_RECT_F& rcs,const D2D1_RECT_F& rce, DWORD tick_distance )
{
	struct st
	{
		WORD magic;
		DWORD starttmm;
		float offr,offt,offb,offl;
		D2D1_RECT_F rcs;
	};

	st* _c = (st*)cxt;
	
	if ( _c == nullptr )
	{
		_c = new st();
		_c->magic = 598;
		_c->starttmm = ticknow;
		_c->rcs = rcs;
		
		_c->offl = (rce.left - rcs.left);
		_c->offr = (rce.right - rcs.right);
		_c->offt = (rce.top - rcs.top);
		_c->offb = (rce.bottom - rcs.bottom);

		*ret = rcs;
	}
	else
	{
		_ASSERT(_c->magic == 598);

		float t = (float)(ticknow - _c->starttmm ) / tick_distance;

		if ( 1.0f <= t )
		{
			*ret = rce;
			delete _c;
			_c = nullptr;
		}
		else
		{
			const double halfpai = 3.14159/2.0;
			auto t2 = sin(halfpai*t);

			ret->top = _c->rcs.top + (float)(_c->offt * t2);
			ret->left = _c->rcs.left + (float)(_c->offl * t2);
			ret->right = _c->rcs.right + (float)(_c->offr * t2);
			ret->bottom = _c->rcs.bottom + (float)(_c->offb * t2);
		}
	}
	return (HANDLE)_c;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// header[0] = "Authorization : xxxx";
// header[0] = "Content-Type : xxxx";

struct InnerComplete
{
	ResponseData* res;
	LPVOID complete;
};


//DLLEXPORT int GETInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret, LPVOID complete )
//{
//	std::map<std::wstring,std::wstring> hd;
//	for( int i = 0; i < headercnt; i++ )
//	{
//		std::wstring s = header[i];
//		int pos = s.find( ':' );
//		if ( pos > 0 )
//		{
//			auto h = s.substr( 0, pos );
//			auto v = s.substr( pos+1, s.length()-(pos+1) );
//		
//			hd[h] = v;
//		}
//	}
//
//	InnerComplete rc1;
//	rc1.complete = complete;
//	rc1.res = ret;
//	
//	ret->seqno = GETInternetEx((LPCWSTR)url, hd, &ret->result, &ret->data, &ret->callback, (LPVOID)&rc1);
//	return ret->seqno;
//}


struct _InnerCallbackObject
{
	ResponseData* sender;
	LPVOID complete;
};
	

static void _innerInetCallback(void* sender, int result, LPCWSTR content, LPCWSTR res_headers, IBinary& res_body) 
{
	typedef void(*cmpl)(void*);

	cmpl complete;
	ResponseData* rd;

	_InnerCallbackObject* p = (_InnerCallbackObject*)sender;
	{
		rd = p->sender;
		complete = (cmpl)p->complete;

		rd->result = result;
	
		BSTR bs;	
		Utf8ToBSTR(res_body, &bs);
	
		rd->data = bs;
	}
	
	delete p;
		
	complete(rd);	
}


DLLEXPORT int WINAPI GETInternet( BSTR url, BSTR headers_CRLF, ResponseData* sender, LPVOID complete )
{
	_InnerCallbackObject* p = new _InnerCallbackObject();
	p->complete = complete;
	p->sender = sender;

	int a = GETInternetEx((LPCWSTR)url, headers_CRLF, p, _innerInetCallback);
	return a;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////
// header[0] = "Content-Type : xxxx";
// header[1] = "Content-Length : xxxx";
DLLEXPORT void WINAPI POSTInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret )
{
	std::map<std::wstring,std::wstring> hd;
	for( int i = 0; i < headercnt; i++ )
	{
		std::wstring s = header[i];
		int pos = s.find( ':' );
		if ( pos > 0 )
		{
			auto h = s.substr( 0, pos );
			auto v = s.substr( pos+1, s.length()-(pos+1) );
		
			hd[h] = v;
		}
	}
	
	//POSTInternetEx((LPCWSTR)url, hd, &ret->result, &ret->data, &ret->callback);
}


DLLEXPORT void WINAPI ResponseDataInit(ResponseData* data)
{
	ZeroMemory(data,sizeof(ResponseData));
}
DLLEXPORT void WINAPI ResponseDataClear(ResponseData* data)
{
	if ( data == nullptr ) return;

	if ( data->callback )
		data->callback->Release();

	if ( data->data )
		::SysFreeString(data->data);
	
	data->result = 0;
}


DLLEXPORT bool WINAPI WriteFileWStore( LPCWSTR fnm, const byte* src, DWORD src_length, int typ )
{
	// fnmはtemporary path以下でないと失敗する。

	if ( fnm == nullptr || src_length == 0 || src == nullptr ) 
		return false;

	std::wstring dirfnm;

	if ( fnm[1] != ':' )
	{
		WCHAR cb[MAX_PATH];
		::GetTempPath(MAX_PATH, cb);

		dirfnm = cb;
		dirfnm += fnm;
	}
	else
		dirfnm  = fnm;
	
	bool bNew  = false;
	CREATEFILE2_EXTENDED_PARAMETERS pms = {0};
	pms.dwSize = sizeof(pms);
	pms.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

	HANDLE h = ::CreateFile2( dirfnm.c_str(), GENERIC_WRITE,0,OPEN_EXISTING,&pms );
	if ( INVALID_HANDLE_VALUE != h )
	{
		LARGE_INTEGER x;
		x.QuadPart = 0;
		SetFilePointerEx(h,x,nullptr, FILE_END);
	}
	else if ( ERROR_FILE_NOT_FOUND == ::GetLastError())
	{
		h = ::CreateFile2( dirfnm.c_str(), GENERIC_WRITE,0,CREATE_ALWAYS,&pms );
		bNew = true;
	}
	
	DWORD dw = 0;

	if ( INVALID_HANDLE_VALUE != h )
	{
		if ( typ == 1 && bNew )
		{
			// BOM utf8
			BYTE bom[] = {0xEF, 0xBB, 0xBF};
			::WriteFile(h, bom, 3, &dw, nullptr );
		}		
		//::WriteFile(h, IBinaryPtr(src), IBinaryLen(src), &dw, nullptr );
		::WriteFile(h, src, src_length, &dw, nullptr );
		::CloseHandle(h);
		return true;
	}
	return false;
}

DLLEXPORT bool WINAPI DateTimeInit(DateTime* datetime)
{
	if (!SystemTimeToFileTime(&datetime->inDatetime, &datetime->ft)) return false;

	datetime->hikaku = datetime->ft.dwHighDateTime;
	datetime->hikaku = (datetime->hikaku << 32) + datetime->ft.dwLowDateTime;
	datetime->string = nullptr;


	DateTime utc = *datetime;

	if (utc.bLocaltime)
	{
		utc = LTCtoUTC(*datetime);
	}

	WCHAR cb[200];

	if (utc.format_string == DATETIME_FORMAT::RFC1123)
	{
		//Thu, 16 May 2013 03:36:09 GMT
		auto& d = utc.inDatetime;

		WCHAR* week[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
		WCHAR* month[] = { L"Jan",L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };

		StringCbPrintf(cb, _countof(cb), L"%s, %02d %s %04d %02d:%02d:%02d GMT", week[d.wDayOfWeek], d.wDay, month[d.wMonth - 1], d.wYear, d.wHour, d.wMinute, d.wSecond);
	}
	else if (utc.format_string == DATETIME_FORMAT::ISO8601)
	{
		auto& d = utc.inDatetime;
		StringCbPrintf(cb, _countof(cb), L"%04d-%02d-%02dT%02d:%02d:%02dZ",
			d.wYear, d.wMonth, d.wDay,
			d.wHour, d.wMinute, d.wSecond);
	}
	else
	{
		// DATETIME_FORMAT::YYYYMMDD
		StringCbPrintf(cb, _countof(cb), L"%04d/%02d/%02d", datetime->inDatetime.wYear, datetime->inDatetime.wMonth, datetime->inDatetime.wDay);
	}

	datetime->string = ::SysAllocString(cb);
	return true;
}
DLLEXPORT void WINAPI Now(DateTime* datetime)
{
	SYSTEMTIME systime = { 0 };
	GetLocalTime(&systime);	//現在日時の取得
	datetime->inDatetime = systime;
	datetime->bLocaltime = true;
	DateTimeInit(datetime);
}
DLLEXPORT bool WINAPI DateTimeParse(LPCWSTR cdate, DateTime* datetime)
{	
	VARIANT d, dst;
	VariantInit(&d);
	d.vt = VT_BSTR;
	d.bstrVal = ::SysAllocString(cdate);
	
	VariantInit(&dst);
	if (S_OK != ::VariantChangeType(&dst, &d, 0, VT_DATE))
		return false;
	
	SYSTEMTIME systime = { 0 };
	if (VariantTimeToSystemTime(d.date, &systime))
	{
		datetime->inDatetime = systime;
		datetime->bLocaltime = true;
		DateTimeInit(datetime);
	}
	else
		return false;
		
	return true;
}
DLLEXPORT DateTime WINAPI LTCtoUTC(const DateTime& datetime)
{
	_ASSERT(datetime.bLocaltime == true);
	SYSTEMTIME t = datetime.inDatetime;
	DateTime ret;
	TzSpecificLocalTimeToSystemTime(NULL, &t, &ret.inDatetime);
	ret.bLocaltime = false;
	ret.format_string = datetime.format_string;
	DateTimeInit(&ret);
	return ret;
}
DLLEXPORT DateTime WINAPI UTCtoLTC(const DateTime& datetime)
{
	_ASSERT(datetime.bLocaltime == false);
	SYSTEMTIME t = datetime.inDatetime;
	DateTime ret;
	SystemTimeToTzSpecificLocalTime(NULL, &t, &ret.inDatetime);
	ret.bLocaltime = true;
	ret.format_string = datetime.format_string;
	DateTimeInit(&ret);
	return ret;
}


}