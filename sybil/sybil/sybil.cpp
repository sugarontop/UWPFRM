#include "pch.h"

#include "sybil.h"
#include "content/d2dmisc.h"
#include "internet.h"


using namespace V4;
namespace sybil {


DLLEXPORT bool CreateTextFromat(IDWriteFactory* wfac, LPCWSTR fontname, float height, int weight, IDWriteTextFormat** tf  )
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

DLLEXPORT bool CreateSingleTextLayout(IDWriteFactory* wfac, LPCWSTR str, int length, IDWriteTextFormat* tf, IDWriteTextLayout** tl  )
{
	return (S_OK == wfac->CreateTextLayout( str,length, tf, 32000,32000, tl ));
}

DLLEXPORT void DrawTextLayoutCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, IDWriteTextLayout* tl, ID2D1Brush* br  )
{	
	DWRITE_TEXT_METRICS tm;
	tl->GetMetrics(&tm);

	FPointF pt;
	
	pt.x = ((rc.right - rc.left) - tm.width)/2.0f;
	pt.y = ((rc.bottom - rc.top) - tm.height)/2.0f;
	
	p->DrawTextLayout( pt, tl, br );
}



DLLEXPORT void DrawTextCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* br  )
{
	auto old = tf->GetTextAlignment();
	tf->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER );	
	p->DrawText( str, length, tf, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	tf->SetTextAlignment( old );
}

DLLEXPORT HANDLE DrawDriftRect( HANDLE cxt, D2D1_RECT_F* ret, DWORD ticknow, const D2D1_RECT_F& rcs,const D2D1_RECT_F& rce, DWORD tick_distance )
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


DLLEXPORT int GETInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret, LPVOID complete )
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

	InnerComplete rc1;
	rc1.complete = complete;
	rc1.res = ret;
	
	ret->seqno = GETInternetEx((LPCWSTR)url, hd, &ret->result, &ret->data, &ret->callback, (LPVOID)&rc1);
	return ret->seqno;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
// header[0] = "Content-Type : xxxx";
// header[1] = "Content-Length : xxxx";
DLLEXPORT void POSTInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret )
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


DLLEXPORT void ResponseDataInit(ResponseData* data)
{
	ZeroMemory(data,sizeof(ResponseData));
}
DLLEXPORT void ResponseDataClear(ResponseData* data)
{
	if ( data == nullptr ) return;

	if ( data->callback )
		data->callback->Release();

	if ( data->data )
		::SysFreeString(data->data);
	
	data->result = 0;
}


DLLEXPORT bool WINAPI WriteFile( LPCWSTR fnm, const byte* src, DWORD src_length, int typ )
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

}