#include "pch.h"
#include "sybil.h"
#include "content/d2dmisc.h"

using namespace sybilchart;
using namespace V4;
//struct ChartData
//{
//	int data_count;
//
//	float* data_x;
//	float* data_y;
//
//	float ymax, ymin;
//	float xmax, xmin;
//	
//
//};

// OnDevelopping




DLLEXPORT void DrawChart( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, ChartData& c )
{
	_ASSERT( rc.right!=rc.left );
	_ASSERT( rc.bottom!=rc.top );

	D2DMat m;

	m._11 = (rc.right-rc.left)/(c.xmax-c.xmin);
	m._22 = -(rc.bottom-rc.top)/(c.ymax-c.ymin);
	m._12 = 0;
	m._21 = 0;	
	m._31 = m._11*c.xmin;
	m._32 = rc.bottom-m._22*c.ymin;

	FPointF ptprv = m.Multi( FPointF(c.data_x[0], c.data_y[0]) );

	ComPTR<ID2D1SolidColorBrush> br;
	p->CreateSolidColorBrush( D2RGB(0,0,0), &br );

	for( int i = 1; i < c.data_count; i++ )
	{
		FPointF pt = m.Multi( FPointF(c.data_x[i], c.data_y[i]) );

		p->DrawLine( ptprv, pt, br );

		ptprv = pt;
	}
}