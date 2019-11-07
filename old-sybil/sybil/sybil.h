#pragma once

#ifdef _WINDLL
#define DLLEXPORT extern "C" __declspec( dllexport )
#else
#define DLLEXPORT extern "C"
#endif

// UWP Frame common library


namespace sybil {

DLLEXPORT bool WINAPI CreateTextFromat(IDWriteFactory* wfac, LPCWSTR fontname, float height, int weight, IDWriteTextFormat** ret);

DLLEXPORT bool WINAPI CreateSingleTextLayout(IDWriteFactory* wfac, LPCWSTR str, int length, IDWriteTextFormat* tf, IDWriteTextLayout** ret );

DLLEXPORT void WINAPI DrawTextCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* clr );
DLLEXPORT void WINAPI DrawTextLayoutCenter( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, IDWriteTextLayout* tl, ID2D1Brush* br  );

DLLEXPORT HANDLE WINAPI DrawDriftRect( HANDLE cxt, D2D1_RECT_F* ret, DWORD ticknow, const D2D1_RECT_F& rcs,const D2D1_RECT_F& rce, DWORD tick_distance );


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
DLLEXPORT void WINAPI ResponseDataInit(ResponseData* data);
DLLEXPORT void WINAPI ResponseDataClear(ResponseData* data);

DLLEXPORT int WINAPI GETInternet( BSTR url, BSTR headers_CRLF, ResponseData* sender, LPVOID complete );
DLLEXPORT void WINAPI POSTInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret );


//////////////////////////////////////////////////////////////////////////////

enum DATETIME_FORMAT { YYYYMMDD = 0,RFC1123,ISO8601 };

struct DateTime
{
	SYSTEMTIME inDatetime;

	FILETIME ft;
	BSTR string;
	DATETIME_FORMAT format_string;
	LONGLONG hikaku;
	bool bLocaltime;
};

DLLEXPORT bool WINAPI DateTimeInit(DateTime* datetime);
DLLEXPORT void WINAPI Now(DateTime* datetime);
DLLEXPORT bool WINAPI DateTimeParse(LPCWSTR cdate, DateTime* datetime);


DLLEXPORT DateTime WINAPI LTCtoUTC(const DateTime& datetime);
DLLEXPORT DateTime WINAPI UTCtoLTC(const DateTime& datetime);




};


