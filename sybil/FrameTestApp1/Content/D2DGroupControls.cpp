#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDriftDialog.h"
#include "D2DSplitControls.h"
using namespace V4;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DGroupControls::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name,WNDTYP wd, int controlid)
{
	_ASSERT( rc.left == 0 );
	_ASSERT( rc.top == 0 );
	
	
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);
	drift_ = nullptr;
	drift_typ_ = 0;
	drift_value_ = 0;


	FRectF menurc(0,0,rc.Width(), 20 );
	D2DTitlebarMenu* menu = new D2DTitlebarMenu(); // topのタイトル下のメニュー
	menu->Create( parent, this, menurc, 0, L"noname", 10 );

	menu_ = menu;


	FRectF leftrc(20,0,rc.Width(), rc.Height() );
	D2DSplitControls* left = new D2DSplitControls();
	left->Create( parent,this, leftrc, VISIBLE,L"noname", 11); // 右左

	center_ = left;
}



D2DControls* D2DGroupControls::GetInner(int id)
{
	for( auto& it : controls_	)
		if ( it->GetId() == id  )
		{
			auto ret = dynamic_cast<D2DControls*>( it.get() );

			return ret;

		}
	return nullptr;
	//return static_cast<D2DSplitControls*>(center_)->GetInner(idx);

}
int D2DGroupControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( !(stat_ & VISIBLE) )
		return 0;
		
	int ret = 0;
	bool bl = true;

	switch( message )
	{
		case WM_PAINT:
		{			
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			mat.Offset(rc_.left, rc_.top);


			DefPaintWndProc(d,message,wp,lp);

			mat.PopTransform();			
			return 0;
		}
		break;
		case WM_D2D_OPEN_SLIDE_MENU:
		{
			auto rc1 = dynamic_cast<D2DMainWindow*>(parent_)->GetMainWndRect();
			if ( drift_ && drift_typ_ == 0)
			{
				FRectF rc( rc1.right, rc1.top, rc1.right, rc1.bottom );
				
				drift_->SetRect(rc);

				rc.left = rc.right - drift_value_;

				drift_->SetDriftRect( rc );	
				
				ret = 1;
			}
			else if ( drift_ && drift_typ_ == 1)
			{
				FRectF rc( rc1.left, rc1.bottom, rc1.right, rc1.bottom );
				
				drift_->SetRect(rc);

				rc.top = rc.bottom - drift_value_;

				drift_->SetDriftRect( rc );	
				
				ret = 1;
			}
		}
		break;
		case WM_D2D_OPEN_SLIDE_MENU_EX:
		{
			auto rc1 = dynamic_cast<D2DMainWindow*>(parent_)->GetMainWndRect();
			SlideMenuItem* s = (SlideMenuItem*)wp;
			drift_typ_ = 0;

			
			this->SetDriftControl(0, 500, s->item);

			

			if ( drift_ && drift_typ_ == 0)
			{
				FRectF rc( rc1.right, rc1.top, rc1.right, rc1.bottom );
				
				drift_->SetRect(rc);

				rc.left = rc.right - drift_value_;

				drift_->SetDriftRect( rc );	
				
				ret = 1;
			}
			else if ( drift_ && drift_typ_ == 1)
			{
				FRectF rc( rc1.left, rc1.bottom, rc1.right, rc1.bottom );
				
				drift_->SetRect(rc);

				rc.top = rc.bottom - drift_value_;

				drift_->SetDriftRect( rc );	
				
				ret = 1;
			}
		}
		break;

		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			auto rc = parent_control_->GetRect().GetContentRect();

			rc_ = rc;

			//if ( ty_ == TYP::HEIGHT_FLEXIBLE )
			//{
			//	rc_.top = rc.top;
			//	rc_.bottom = rc.bottom;
			//}
			//else if ( ty_ == TYP::WIDTH_FLEXIBLE )
			//{
			//	rc_.left = rc.left;
			//	rc_.right = rc.right;
			//}
		}
		break;
		case WM_KEYUP:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Control:
				{	
					if ( menu_->IsHide())
					{
						SetCapture(menu_);						
					}
					else
					{
						_ASSERT( menu_ == GetCapture() );
						
						ReleaseCapture();
												
					}
					d->redraw();
					ret = 1;
					bl = false;
				}
				break;

			}


		}
		break;
	}

	if ( bl )
		return D2DControls::DefWndProc(d,message,wp,lp);
	return ret;
}
void D2DGroupControls::SetDriftControl( int typ, float drift_value, D2DControls* ctrls )
{	
	if ( drift_ == nullptr )
	{
		drift_typ_ = typ;
		drift_value_ = drift_value;

		drift_ = new D2DDriftDialog(); 
		drift_->Create( parent_, this, FRectF(0,0,0,0), VISIBLE,L"noname" );

	}

	ctrls->ParentExchange( drift_ );
}
