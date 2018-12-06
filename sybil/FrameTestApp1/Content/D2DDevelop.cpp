#include "pch.h"
#include "D2DDevelop.h"
#include "D2DWindowMessage.h"
#include "higgsjson.h"


using namespace HiggsJson;

namespace V4 {

int D2DList::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide())
		return 0;
	int ret = 0;
		
	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = *(d->cxt());

			D2DMatrix mat(cxt);	
		
			mat_ = mat.PushTransform();
			FRectF rcborder = rc_.GetBorderRect();

			cxt.cxt->FillRectangle( rcborder, cxt.bluegray );

			mat.Offset( rcborder.left, rcborder.top );		
			
			
			DrawList( cxt );
			

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_PROPERTY_LIST_JSON :
		{
			D2DJson* info = (D2DJson*)wp;
			D2DControl* crl = (D2DControl*)info->sender;
			BSTR json = info->json;

			Parse(json);

			d->redraw();
			ret = 1;
		}
		break;
	}


	return ret;

}
void D2DList::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	Item m;
	m.title = L"D2DList";
	m.value = CStrHex( (DWORD)this);
	m.typ = 0;
	ar_.push_back(m);

}
void D2DList::Parse(BSTR json)
{
	std::vector<Higgs> ar;
	ParseList(json, ar );

	//‚Ü‚¾“r’†


}
void D2DList::DrawList(D2DContext& cxt )
{
	FRectF rct(0,0,200,26 );
	FRectF rcv(200,0,400,26 );

	for( auto& it : ar_ )
	{
		cxt.cxt->DrawText( it.title.c_str(), it.title.length(), cxt.textformat, rct, cxt.white );

		cxt.cxt->DrawText( it.value.c_str(), it.value.length(), cxt.textformat, rcv, cxt.white );

		FPointF pt1,pt2;
		pt1.y = pt2.y = rct.bottom;
		pt2.x = rcv.right;
		cxt.cxt->DrawLine( pt1, pt2, cxt.white );

		rct.Offset( 0, 30 );
		rcv.Offset( 0, 30 );

	}


}

////////////////////////////////////////////////////////////////////////////////////////


















};