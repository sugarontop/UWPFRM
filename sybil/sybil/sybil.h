#pragma once

#ifdef _WINDLL
#define DLLEXPORT extern "C" __declspec( dllexport )
#else
#define DLLEXPORT extern "C"
#endif

// UWP Frame common library


namespace sybil {

DLLEXPORT bool CreateTextFromat(IDWriteFactory* wfac, LPCWSTR fontname, float height, int weight, IDWriteTextFormat** ret);

DLLEXPORT bool CreateSingleTextLayout(IDWriteFactory* wfac, LPCWSTR str, int length, IDWriteTextFormat* tf, IDWriteTextLayout** ret );

DLLEXPORT void DrawTextCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* clr );
DLLEXPORT void DrawTextLayoutCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, IDWriteTextLayout* tl, ID2D1Brush* br  );

DLLEXPORT HANDLE DrawDriftRect( HANDLE cxt, D2D1_RECT_F* ret, DWORD ticknow, const D2D1_RECT_F& rcs,const D2D1_RECT_F& rce, DWORD tick_distance );


DLLEXPORT bool WINAPI WriteFileWStore( LPCWSTR fnm, const byte* src, DWORD src_length, int typ );
//////////////////////////////////////////////////////////////////////////////
struct ResponseData
{
	int seqno;
	int result;
	BSTR data;
	IXMLHTTPRequest2Callback* callback;
	void* option;


};
DLLEXPORT void ResponseDataInit(ResponseData* data);
DLLEXPORT void ResponseDataClear(ResponseData* data);

DLLEXPORT int GETInternet( BSTR url, BSTR headers_CRLF, ResponseData* sender, LPVOID complete );
DLLEXPORT void POSTInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret );
};


