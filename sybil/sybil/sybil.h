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



struct ResponseData
{
	int result;
	BSTR data;
	IXMLHTTPRequest2Callback* callback;
};

DLLEXPORT void GETInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret );
DLLEXPORT void POSTInternet( BSTR url, BSTR* header,int headercnt, ResponseData* ret );
};


namespace sybilchart {

struct ChartData
{
	int data_count;

	float* data_x;
	float* data_y;

	float ymax, ymin;
	float xmax, xmin;
	

};


DLLEXPORT void DrawChart( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, ChartData& cdata );


};