#include "pch.h"
#include "D2DUniversalControl.h"
#include "sybil.h"
#include "content/D2DWindowMessage.h"
using namespace V4;
using namespace sybil;





D2DCombobox::D2DCombobox()
{

}
void D2DCombobox::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
	Clear();
	ListboxHeight_ = 300;
	md_ = 0;
	floating_relative_idx_ = -1;
	start_idx_ = 0;
	
}
int D2DCombobox::AddItem( LPCWSTR key, LPCWSTR value )
{
	KeyValue x;
	x.key = key;
	x.value = value;
	items_.push_back(x);

	return items_.size();
}
void D2DCombobox::Clear()
{
	items_.clear();
	select_idx_ = -1;
}
void D2DCombobox::OnReleaseCapture(int layer)
{
	D2DControl::OnReleaseCapture(layer);
}

int D2DCombobox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( (stat_ & STAT::VISIBLE )== 0 ) 
		return 0;

	int ret = 0;
	bool bl = false;
	
	switch( message )
	{
		case WM_PAINT:
		{
			DrawSelectedText(d);

			if ( IsCaptured())
				DrawListbox(d);
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);


			FRectF rclistbox = rc_;
			rclistbox.top = rc_.bottom;
			rclistbox.bottom = rclistbox.top + ListboxHeight_;

			if ( rc_.PtInRect(pt ))
			{
				parent_control_->SetCapture(this);

				d->redraw();
				ret = 1;
				floating_relative_idx_ = -1;

				md_ = 1; // show listbox
				ret = 1;
			}
			else if ( rclistbox.PtInRect(pt))
			{
				md_ = 2; // select in listbox
				ret = 1;
			}
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( md_ == 1 )
			{
				FPointF pt = mat_.DPtoLP(lp);

				FRectF rclistbox = rc_;
				rclistbox.top = rc_.bottom;
				rclistbox.bottom = rclistbox.top + ListboxHeight_;

				int new_floating_idx = floating_relative_idx_;
				if ( rclistbox.PtInRect(pt))
				{										
					new_floating_idx = (int)((pt.y - rclistbox.top) / 30.0f);
								
				}

				if ( new_floating_idx != floating_relative_idx_ )
				{
					d->redraw();	
					floating_relative_idx_ = new_floating_idx;
				}

				ret = 1;
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( md_ == 2 && this == parent_control_->GetCapture())
			{								
				parent_control_->ReleaseCapture();
				d->redraw();
				ret = 1;

				
				_ASSERT( (stat_ & STAT::CAPTURED) == 0 );

				select_idx_ = floating_relative_idx_ + start_idx_;
				OnSelectChanged();
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			if ( parent_control_->GetCapture() == this  )
			{
				switch( arg->VirtualKey )
				{
					case Windows::System::VirtualKey::Escape:
					{						
						parent_control_->ReleaseCapture();
						d->redraw();
						ret = 1;
					}
				}				
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			if ( md_ == 1 && this == parent_control_->GetCapture())
			{
				Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;

				int delta = arg->CurrentPoint->Properties->MouseWheelDelta;
				ret = 1;
				d->redraw();
				start_idx_ = max(0,min((int)items_.size()-1, (delta < 0 ? (int)start_idx_+1 : (int)start_idx_-1 )));

				OnSelectChanged();
			}
		}
		break;
	}


	return ret;
}
void D2DCombobox::DrawSelectedText(D2DWindow* d)
{
	auto& cxt = *(d->cxt());
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();
	mat.Offset(rcb.left, rcb.top);

	auto rc1 = rcb.ZeroRect();
	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );

	std::wstring title;

	if ( select_idx_ == -1 )
		title = L"please select";
	else if ( 0 <= select_idx_ && select_idx_ < (int)items_.size())
	{
		title = items_[select_idx_].value;
	}

	sybil::DrawTextCenter( cxt.cxt, rc1, title.c_str(), title.length(),cxt.textformat, cxt.black  ); 

	mat.PopTransform();

}
void D2DCombobox::DrawListbox(D2DWindow* d)
{
	auto& cxt = *(d->cxt());
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();


	mat.Offset(rcb.left, rcb.top);

	auto rc1 = rcb.ZeroRect();
	


	rc1.Offset( 0, rc1.Height());
	rc1.bottom = rc1.top + ListboxHeight_;

	D2DRectFilter fil( cxt, rc1.GenInflateRect(1,1) );

	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );


	rc1.bottom = rc1.top + 30;
	int ridx = 0;
	for( UINT i = start_idx_; i < items_.size(); i++ )
	{
		if ( floating_relative_idx_ == ridx )
		{
			cxt.cxt->FillRectangle( rc1, cxt.halftone );
		}
		auto& it = items_[i];
		sybil::DrawTextCenter( cxt.cxt, rc1, it.value.c_str(), it.value.length(),cxt.textformat, cxt.black  ); 
		rc1.Offset( 0, 30 );
		ridx++;
	}


	mat.PopTransform();

}
void D2DCombobox::OnSelectChanged()
{
	wparam wp;
	wp.cmb = this;

	if ( OnSelectChanged_ )
		OnSelectChanged_(this);
	else
		parent_->SendMessage( WM_D2D_COMBOBOX_CHANGED, (INT_PTR)&wp, nullptr );

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//D2DTest::D2DTest()
//{
//	bfresize_ = false;
//	cc_ = nullptr;
//	mode_ = 0;
//}
//void D2DTest::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
//{
//	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
//
//	minmum_v_ = [this](D2DTest*sender, D2DContext& cxt) {
//		
//		cxt.cxt->FillRectangle( rc_, cxt.white );
//		cxt.cxt->DrawText( L"minum", 5, cxt.cxtt.textformat, rc_, cxt.black );
//
//	};
//	default_v_ = [this](D2DTest*sender, D2DContext& cxt) {
//		
//		ComPTR<ID2D1SolidColorBrush> br;
//		cxt.cxt->CreateSolidColorBrush( D2RGBA(200,200,200,100), &br );
//
//		cxt.cxt->FillRectangle( rc_, br );
//	};
//
//
//	D2DButton* b1 = new D2DButton();
//	b1->CreateButton( parent, this, FRectF(0,10,100,30),VISIBLE,L"a1", L"noname");
//	b1->OnClick_ = [this](D2DButton* b)
//	{
//
//		this->dstRect_ = FRectF( 1500,0,FSizeF(0,1200));
//		this->bfresize_ = true;
//
//	};
//
//
//	FRectF rc3( 100,100, FSizeF(100, 30));
//	 b1 = new D2DButton();
//	b1->CreateButton(parent,this, rc3,VISIBLE,L"a1",L"noname" );	
//	b1->OnClick_ = [parent, this](D2DButton*b)
//	{
//		D2DMessageBox d;
//		d.Show(parent, this, FRectF(100,100,FSizeF(500,200)), L"test", L"messagebox" );
//	};
//
//
//}
//int D2DTest::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
//{
//	int ret = 0;
//	if ( !(stat_ & VISIBLE) )
//		return ret;
//	switch( message )
//	{
//
//		case WM_PAINT:
//		{
//			auto& cxt = *(d->cxt());
//
//			D2DMatrix mat(cxt);
//			mat_ = mat.PushTransform();
//
//			FRectF rcb = rc_.GetBorderRect();
//			//mat.Offset(rcb.left, rcb.top); // border rectŠî€
//						
//						
//			FRectF rca = rc_.GetContentBorderBase(); 
//
//			
//
//			if ( bfresize_ || cc_ !=nullptr )
//			{
//				FRectF rcz = dstRect_;
//				FRectF rc;
//				cc_ = DrawDriftRect( cc_, &rc, cxt.tickcount_, rc_, rcz, 500 );
//				if ( cc_ == nullptr )
//				{					
//					bfresize_ = false;
//					mode_ = ( dstRect_.Size().width == 0 ? 0 : 1);
//
//					if ( mode_ == 0 )
//					{
//						if ( ParentControl()->GetCapture() == this )
//							this->ParentControl()->ReleaseCapture();
//					}
//				}
//
//				rc_ = rc;
//
//				default_v_(this, cxt );
//
//				//cxt.cxt->DrawRectangle( rc, cxt.black );
//				d->redraw();
//
//			}
//			else			
//			{
//				if ( mode_ == 0 )
//					minmum_v_(this, cxt );
//				else
//					default_v_(this, cxt );
//
//				FRectF rcb = rc_.GetBorderRect();
//				mat.Offset(rcb.left, rcb.top);
//
//				if ( rcb.Size().width > 0 )
//					D2DControls::DefWndProc(d,message,wp,lp);
//
//				
//			}
//
//			mat.PopTransform();
//			return 0;
//		}
//		break; 
//		case WM_MOUSEMOVE:
//		{
//			/*if ( rc_.Size().width <= 0 )
//			{
//				bool bl = ( this->ParentControl()->GetCapture() == this );
//
//				if ( bl )
//					this->ParentControl()->ReleaseCapture();
//			}*/
//
//			ret = D2DControls::DefWndProc(d,message,wp,lp);
//		}
//
//		break;
//		default:
//			ret = D2DControls::DefWndProc(d,message,wp,lp);
//		break;
//	}
//	if ( ret == 0 )
//		ret = ( this->parent_control_->GetCapture() == this ? 1 : 0 );
//
//	return ret;
//}
//




