#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"

using namespace V4;


void WhiteBack( D2DContext& cxt, D2D1_RECT_F& rc )
{
	cxt.cxt->FillRectangle(rc, cxt.white);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


D2DControl::D2DControl()
{
	disp_ = nullptr; 
	parent_ = nullptr;

	back_ground_ = WhiteBack;

}
D2DControl::~D2DControl()
{
	if ( disp_ )
	{
		disp_->Release();
		disp_ = nullptr;
	}
}

int D2DControl::WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{	
	return 0;
}


void D2DControl::InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	_ASSERT(parent);
	
	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	parent_control_ = pacontrol;
	name_ = name;
	id_ = controlid;
	disp_ = nullptr; 

	

	if (parent_control_)
	{
		auto target = std::shared_ptr<D2DControl>(this);
		parent_control_->controls_.push_back( target ); 
	}

	auto mainparent = dynamic_cast<D2DMainWindow*>(parent);
	if ( mainparent )
	{
		if ( name_ != L"noname" && name_ != NONAME )
			mainparent->hub_[ name_ ] = this;
	}	
}
void D2DControl::DestroyControl()
{
	_ASSERT( parent_control_->GetCapture() != this );

	WndProc(parent_, WM_DESTROY, 0, nullptr );
	parent_->redraw();

	// ループ内で実行する場合は、必ず１を返すこと
	if ( !(stat_ & STAT::DEAD ) )
	{		
		stat_ &= ~STAT::VISIBLE;
		stat_ |= STAT::DEAD;


		if ( parent_control_ )
		{
			auto p = parent_control_->Detach( this );

			parent_->AddDeath(p);
		}

		if ( disp_ )
		{
			disp_->Release();
			disp_ = nullptr;
		}
	}
}
bool D2DControl::IsCaptured() const
{
	//return (this == parent_control_->GetCapture());

	return ( (stat_ & STAT::CAPTURED) > 0 );
}
void D2DControl::DoCapture()
{
	parent_control_->SetCapture(this);
}
D2DControls* D2DControl::ParentExchange( D2DControls* newparent )
{
	auto t = parent_control_->Detach(this);
	newparent->controls_.push_back(t);
	auto r = parent_control_;
	parent_control_ = newparent;
	return r;
}
void D2DControl::UnActivate()
{
	if ( GetParentControl()->GetCapture() == this )
		GetParentControl()->ReleaseCapture(this);
}
void D2DControl::OnSetCapture(int layer)
{
	stat_ |= STAT::CAPTURED;
}
void D2DControl::OnReleaseCapture(int layer)
{
	stat_ &= ~STAT::CAPTURED;
}
void D2DControl::SetCapuredLock(bool lock )
{
	// この機能は動作しない？

	if ( lock )
		stat_ |= STAT::CAPTURED_LOCK;
	else
		stat_ &= ~STAT::CAPTURED_LOCK;
}
