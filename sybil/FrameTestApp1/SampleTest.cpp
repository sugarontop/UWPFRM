#include "pch.h"
#include "sybil.h"
#include "content/httputil.h"
#include "content/D2DUniversalControl.h"
#include "content/D2DWindowMessage.h"
#include "sampletest.h"
using namespace sybil;


namespace V4 {

class D2DRectbox : public D2DControls
{
	public :
		D2DRectbox(){};
		~D2DRectbox();
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);

		void DrawInner();
		void ParseData( BSTR data );


		sybilchart::ChartData cdata_;
		
};


D2DRectbox::~D2DRectbox()
{
	delete [] cdata_.data_x;
	delete [] cdata_.data_y;
}

void D2DRectbox::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent, pacontrol, rc, stat, name, local_id);


	sybilchart::ChartData c;
	c.data_count = 3;

	float* x = new float[3]; 	
	c.data_x = x;
	for( float a : { 0,10,20 } ) *x++ = a;
	
	float* y = new float[3];
	c.data_y = y;
	for( float a : { 0,100,50 } ) *y++ = a;
	
	c.data_count = 3;
	c.xmax = 20;
	c.xmin = 0;
	c.ymax = 100;
	c.ymin = 0;

	cdata_ = c;
}
int D2DRectbox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() ) 
		return 0;

	int ret = 0;

	static FPointF ptold;

	switch (message )
	{
		case WM_PAINT:
		{
			DrawInner();
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);

			if ( rc_.PtInRect(pt3 ))
			{				
				parent_control_->SetCapture( this );
				ret = 1;
				ptold = pt3;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( IsCaptured()) 
			{				
				FPointF pt = mat_.DPtoLP(lp);

				rc_.Offset( pt.x - ptold.x, pt.y - ptold.y );
				ptold = pt;
				
				d->redraw();
				ret = 1;
			}

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( IsCaptured()) 
			{
				parent_control_->ReleaseCapture();

				ret = 1;
			}

		}
		break;
		case WM_D2D_INTERNET_GET_COMPLETE:
		{			
			XST* x = (XST*)wp;
			
			BSTR b = x->data;

			ParseData(b);

			d->redraw();

			ret = 1;

		}
		break;
	}
	return ret;
}

void D2DRectbox::DrawInner()
{
	auto& cxt = *(parent_->cxt());

	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	mat.Offset(rc_.left, rc_.top);

	FRectF rc = rc_.GetBorderRectZero();

	cxt.cxt->DrawRectangle( rc, cxt.black );
	cxt.cxt->FillRectangle( rc, cxt.white );

	
	std::wstring s = L"movable";

	V4::CenterTextOut( cxt.cxt, rc, s.c_str(), s.length(), cxt.textformat, cxt.bluegray );

	sybilchart::DrawChart( cxt.cxt, FRectF( 0,100, FSizeF(200,200)), cdata_ );

	mat.PopTransform();
}

void D2DRectbox::ParseData( BSTR data )
{
	int len = ::SysStringLen(data);

	int s = 0, e = 0;

	std::vector<float> ar;
	WCHAR* wchar_data =(WCHAR*)data;

	for( int i = 0; i < len; i++ )
	{
		WCHAR ch = (WCHAR)data[i];

		if ( ch == L'\t' && s == 0)
		{
			s = i+1;
		}
		else if ( ch == L'\n' && e == 0)
		{
			e = i;

			std::wstring xs( &wchar_data[s], &wchar_data[e] );

			ar.push_back( (float)_wtof(xs.c_str()) );

			s = e = 0;
		}
	}

	delete [] cdata_.data_x;
	delete [] cdata_.data_y;

	sybilchart::ChartData c;
	c.data_count = ar.size();
	c.xmax = ar.size();
	c.xmin = 0;

	c.data_x = new float[ c.data_count ];
	c.data_y = new float[ c.data_count ];


	c.ymax = ar[0];
	c.ymin = ar[0];

	for( int i = 0; i < c.data_count; i++ )
	{
		float y = ar[i];
		c.data_x[i] = i;
		c.data_y[i] = y;

		_ASSERT( y!=0 );

		c.ymax = max( c.ymax, y );
		c.ymin = min( c.ymin, y );
	}

	cdata_ = c;


}


};



int CreateRectBox( V4::D2DControls* ctrl, const FRectFBM& rc )
{
	V4::D2DRectbox* b = new V4::D2DRectbox();
	b->Create( ctrl->GetParentWindow(), ctrl, rc, V4::STAT::VISIBLE,L"noname" );

	return 0;

}