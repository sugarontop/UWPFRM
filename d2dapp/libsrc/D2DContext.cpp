#include "pch.h"
#include "D2DContext.h"


using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::Storage::Pickers;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Web::Http;
using namespace concurrency;
using namespace Windows::Web;

using namespace Windows::Foundation;

using namespace V4;


/////////////////////////////////////////////

SingletonD2DInstance& SingletonD2DInstance::Init()
{
	static SingletonD2DInstance st;

	if (st.factory.p == NULL)
	{
		D2D1_FACTORY_OPTIONS options;
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	}
	return st;


}

D2DContext::D2DContext()
{
	tickcount_ = 0;
	insins = nullptr;
	free_space = nullptr;
}
D2DContext::D2DContext(const D2DContext& cxt)
{
	// ng D2DContext cxt =  ...
	// ok D2DContext& cxt =  ...
	_ASSERT(0==1);

}



void D2DContext::Init(SingletonD2DInstance& ins, ComPTR<ID2D1Factory> fac, ComPTR<IDWriteFactory> wfac, ComPTR<ID2D1DeviceContext> cxt ) //   const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	tickcount_ = 0;
	insins = &ins;
	HRESULT hr = NOERROR;

	hr = fac->CreateDrawingStateBlock(&m_stateBlock); 
	ASSERT_NOERROR(hr);

	ins.wrfactory = wfac;
	ins.factory = fac;
	
	hr = wfac->CreateTextFormat(
		DEFAULTFONT,
		nullptr,
		DEFAULTWEIGHT,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		DEFAULTFONT_HEIGHT,
		DEFAULTLOCALE,
		&ins.textformat
	);

	ASSERT_NOERROR(hr);

	this->cxt = cxt; 
	
	cxtt.textformat = ins.textformat;
	cxtt.wfactory = ins.wrfactory;

	this->textformat = ins.textformat;
	this->wfactory = ins.wrfactory;

	float height[] = {12,18,22};

	for( int i = 0; i < _countof(height); i++ )
	{
		hr = wfac->CreateTextFormat(
			DEFAULTFONT,
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			height[i],
			DEFAULTLOCALE,
			&textformats[i]
		);

		ASSERT_NOERROR(hr);
	}


	hr = fac->CreateStrokeStyle(
        D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_ROUND,
            D2D1_LINE_JOIN_MITER,
            10.0f,
            D2D1_DASH_STYLE_DASH,
            0.0f),
        nullptr,
        0,
        &dot2_
        );

	ASSERT_NOERROR(hr);


}



void D2DContext::CreateDeviceResources(ID2D1RenderTarget* rt)
{
	// D2DWindow::CreateD2DWindowのタイミングで実行
	// OnDeviceRestoredのタイミングで実行

	rt->CreateSolidColorBrush(D2RGB(0, 0, 0), &black);
	rt->CreateSolidColorBrush(D2RGB(255, 255, 255), &white);
	rt->CreateSolidColorBrush(D2RGB(210, 210, 210), &gray);
	rt->CreateSolidColorBrush(D2RGB(255, 0, 0), &red);
	rt->CreateSolidColorBrush(D2RGB(245, 246, 247), &ltgray);
	rt->CreateSolidColorBrush(D2RGB(218, 219, 220), &ltgray2);
	rt->CreateSolidColorBrush(D2RGB(113, 113, 130), &bluegray);
	rt->CreateSolidColorBrush(D2RGBA(0, 0, 0, 0), &transparent);

	rt->CreateSolidColorBrush(D2RGBA(0, 100, 250, 50), &halftone);
	rt->CreateSolidColorBrush(D2RGBA(250, 113, 130, 150), &halftoneRed);

	rt->CreateSolidColorBrush(D2RGBA(255, 242, 0, 255), &tooltip);
	rt->CreateSolidColorBrush(D2RGBA(255, 242, 0, 255), &yellow);
	rt->CreateSolidColorBrush(D2RGBA(0, 0, 242, 255), &blue);
	rt->CreateSolidColorBrush(D2RGBA(33, 115, 70, 255), &green);

	rt->CreateSolidColorBrush(D2RGB(197, 212, 242), &frame[0]);
	rt->CreateSolidColorBrush(D2RGB(160, 160, 160), &frame[1]);
	rt->CreateSolidColorBrush(D2RGB(105, 105, 105), &frame[2]);
	
	
	auto clr1 = D2RGBA(200,200,200,0);
	auto clr2 = D2RGBA(200,200,200,200);

	D2D1_GRADIENT_STOP gradientStops[] = {{ 0.0f, clr1},{ 1.0f, clr2 }};

	ComPTR<ID2D1GradientStopCollection> gradientStopsCollection;
	rt->CreateGradientStopCollection(gradientStops,_countof(gradientStops),&gradientStopsCollection);
	
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties = D2D1::LinearGradientBrushProperties(FPointF(0,0), FPointF( 0, 30));
	rt->CreateLinearGradientBrush(properties, gradientStopsCollection,  &silver_grd);
	


	// D2DContext::DestroyRenderTargetResourceを忘れないこと
}
void D2DContext::DestroyRenderTargetResource()
{
	ltgray.Release();
	ltgray2.Release();
	black.Release();
	white.Release();
	red.Release();
	gray.Release();
	bluegray.Release();
	transparent.Release();
	halftone.Release();
	halftoneRed.Release();
	tooltip.Release();
	silver_grd.Release();
	yellow.Release();
	blue.Release();
	green.Release();

	frame[0].Release();
	frame[1].Release();
	frame[2].Release();
}
