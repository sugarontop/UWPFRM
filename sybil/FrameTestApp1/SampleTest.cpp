#include "pch.h"
#include "sybil.h"
#include "content/httputil.h"
#include "content/D2DUniversalControl.h"
#include "sampletest.h"
using namespace sybil;


namespace V4 {

class D2DRectbox : public D2DControls
{
	public :
		D2DRectbox(){};
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);

		void DrawInner();

		
};


void D2DRectbox::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent, pacontrol, rc, stat, name, local_id);
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

	
	std::wstring s = L"movable Rectangle";

	V4::CenterTextOut( cxt.cxt, rc, s.c_str(), s.length(), cxt.textformat, cxt.bluegray );
	
	sybilchart::ChartData c;
	c.data_count = 3;

	float x[] = { 0,10,20 };
	float y[] = { 0,100,50 };

	c.data_x = x;
	c.data_y = y;
	c.data_count = 3;
	c.xmax = 20;
	c.xmin = 0;
	c.ymax = 100;
	c.ymin = 0;
	
	sybilchart::DrawChart( cxt.cxt, FRectF( 0,100, FSizeF(200,200)), c );

	mat.PopTransform();
}

};

int CreateRectBox( V4::D2DControls* ctrl, const FRectFBM& rc )
{
	V4::D2DRectbox* b = new V4::D2DRectbox();
	b->Create( ctrl->GetParentWindow(), ctrl, rc, V4::STAT::VISIBLE,L"noname" );

	return 0;

}