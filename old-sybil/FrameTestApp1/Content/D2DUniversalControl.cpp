#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DTextbox.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTextbox.h"
#include "D2DDriftDialog.h"
#include "D2DSplitControls.h"
using namespace V4;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DButton::DefaultDrawButton( D2DButton* sender, D2DContext& cxt )
{
	FRectF rc = sender->rc_.GetBorderRectZero();
	//cxt.cxt->DrawRectangle( rc, cxt.black );
	//cxt.cxt->FillRectangle( rc, cxt.gray );

	//if ( sender->mode_ == 1 )
	//{
	//	cxt.cxt->FillRectangle( rc, cxt.bluegray );	

	//}

	LPCWSTR str = sender->title_.c_str();
	int strlen = sender->title_.length();
	
	ID2D1LinearGradientBrush* br = cxt.silver_grd;
	
	br->SetStartPoint( FPointF(0,rc.top));
	br->SetEndPoint( FPointF(0,rc.bottom));

	ComPTR<ID2D1SolidColorBrush> br2;
	cxt.cxt->CreateSolidColorBrush(D2RGBA(230,230,230,200 ), &br2 );
	
	FillRectangle( cxt, rc, br2 );

	FRectF rc2(rc);
	rc2.InflateRect( -2,-2);

	FillRectangle( cxt, rc2, br );

	CenterTextOut( cxt.cxt, rc, str, strlen, cxt.textformats[0], cxt.black );
}

void D2DButton::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int controlid)
{
	InnerCreateWindow(pacontrol,rc,stat,name, controlid);
	mode_ = 0;
	title_ = title;
	OnPaint_ = DefaultDrawButton;

	

}

int D2DButton::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( !(stat_ & VISIBLE) )
		return 0;
	int ret = 0;

	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);


			OnPaint_( this, cxt );


			mat.PopTransform();
		}
		break; 
		case WM_LBUTTONDOWN:
		{
			Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;
			
			FPointF pt(arg->CurrentPoint->Position);
			
			FPointF pt3 = mat_.DPtoLP(pt);

			mode_ = 0;

			if ( rc_.PtInRect(pt3 ))
			{
				mode_ = 1;
				parent_control_->SetCapture( this );
				ret = 1;

				auto x = GetParentWindow();				
			}

			
		}
		break;
		case WM_KEYDOWN :
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			if ( arg->VirtualKey == Windows::System::VirtualKey::Escape && parent_control_->GetCapture() == this )
			{
				parent_control_->ReleaseCapture();
				mode_ = 0;				
			}
		}
		break;
		case WM_LBUTTONUP :
		{
			if ( parent_control_->GetCapture() == this )
			{
				parent_control_->ReleaseCapture(this, -1);
				mode_ = 0;
				ret = 1;

				Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;

				FPointF pt;
				pt.x = arg->CurrentPoint->Position.X;
				pt.y = arg->CurrentPoint->Position.Y;
			
				FPointF pt3 = mat_.DPtoLP(pt);
							
				if ( rc_.PtInRect(pt3 ))
				{
					if ( OnClick_ )
						OnClick_( this );
				}
			}
		}
		break;

	}
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DTitlebarMenu::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow(pacontrol,rc,stat,name, controlid);

	floating_idx_ = -1;
	for( int i = 0; i < 1; i++ )
	{
		items_.push_back( FRectF(0,0,100,20 ));

		D2DVerticalMenu* x = new D2DVerticalMenu();
		
		FRectF rc(0,0,200,300);
		x->Create( this, rc, 0, L"noname" );		

	}
	
}
void D2DTitlebarMenu::OnReleaseCapture(int layer)
{
	D2DControls::OnReleaseCapture(layer);

	Hide();
}
void D2DTitlebarMenu::OnSetCapture(int layer)
{
	D2DControls::OnSetCapture(layer);

	Visible();
}


int D2DTitlebarMenu::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	switch( message )
	{		
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			auto sz = parent_control_->GetRect().Size();
			rc_.right = rc_.left + sz.width;
		}
		break;
	}

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



			cxt.cxt->FillRectangle( rc_.ZeroRect(), cxt.ltgray ); // DRGB( D2RGB(200,200,200)));

			{
				int j = 0;
				FRectF rc(0,0,0,0);
				for( auto& it  : items_ )
				{
					rc.SetSize( it.Size());
					cxt.cxt->DrawText( L"item", 4, cxt.textformat, rc, cxt.black );
					if ( j++ == floating_idx_ )
						cxt.cxt->FillRectangle( rc, cxt.black); // DRGB( D2RGBA(0,255,0,100) ));

					rc.Offset( it.Size().width, 0 );
					
				}
			}

			//cxt.cxt->FillRectangle( FRectF(-10,-10,10,10), cxt.black );

			DefPaintWndProc(d,message, wp, lp);

			mat.PopTransform();
			return 0;
		}
		break;	
		case WM_MOUSEMOVE:
		{
			if ( this->GetCapture() == nullptr )
			{
				FPointF pt3 = mat_.DPtoLP(lp);
				if ( rc_.PtInRect(pt3))
				{
					pt3.x -= rc_.left;
					FRectF rc(0,0,0,0);
					int j = 0;
					int nf = floating_idx_;
					for( auto& it  : items_ )
					{
						rc.SetSize( it.Size());
						if ( rc.PtInRect(pt3) )
						{
							nf = j;
							break;
						}
						rc.Offset( it.Size().width, 0 );
						j++;
					}

					if ( nf != floating_idx_ )
					{
						floating_idx_ = nf;
						d->redraw();
						ret = 1;
					}
				}
				else if ( floating_idx_ > -1 )
				{
					floating_idx_ = -1;

					for(auto& it : controls_ ) it->Hide();

					d->redraw();
				}
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			selected_idx_ = -1;
			if ( rc_.PtInRect(pt3))
			{
				for(auto& it : controls_ ) it->Hide();

				if ( -1 < floating_idx_ && floating_idx_ < (int)controls_.size() )
				{
					FRectF rc(0,rc_.bottom,0,rc_.bottom);
					for( int i = 0; i < floating_idx_; i++ )
						rc.Offset( items_[i].Width(), 0 );
				
					auto t = controls_[floating_idx_];
				

					t->Visible();
					auto trc = t->GetRect();
					t->SetRect( FRectF(rc.left, rc.bottom, trc.Size()));
				
					selected_idx_ = floating_idx_;
					ret = 1;
				}
			}
			else if ( GetCapture() )
			{
				//ret = HideMenu();
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( 0 <= selected_idx_ && selected_idx_ < (int)controls_.size())
			{
				auto t = controls_[selected_idx_];
				SetCapture(t.get());

				ret = 1;
			}
		}
		break;

		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Escape:
				{	
					ret = HideMenu();

					
				}
				break;

			}
		}
		break;
	}


	if ( bl && ret == 0)
		ret = DefWndProc(d,message,wp,lp);

	return ret;
}
int D2DTitlebarMenu::HideMenu()
{
	int ret = 0;
	if ( GetCapture())
	{
		D2DVerticalMenu* x = dynamic_cast<D2DVerticalMenu*>( GetCapture());
		x->Hide();
		ReleaseCapture();
		parent_->redraw();
		ret = 1;
		floating_idx_ = -1;
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DVerticalMenu::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid )
{
	InnerCreateWindow(pacontrol,rc,stat,name, controlid);


#ifdef _DEBUG
	LPCWSTR json1 = L"[{\"name\":\"hoi\", \"id\":1}, {\"name\":\"hoi2\", \"id\":2}, {\"name\":\"close\", \"id\":9999}]]"; // dumy

	BSTRPtr bjson = json1;
	ParseMenu(bjson);
#endif
	target_ = nullptr;
	pacontrol->SetCapture(this);



}

int D2DVerticalMenu::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() )
		return 0;
	
	int ret = 0;

	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);
						
			cxt.cxt->DrawRectangle( rc_.ZeroRect(), cxt.black );
			cxt.cxt->FillRectangle( rc_.ZeroRect(), cxt.white );

			for( auto& it : items_ )
			{
				DrawItem(cxt, it);
			}


			mat.PopTransform();
			return 0;
		}
		break;
		case WM_MOUSEMOVE :
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt3))
			{
				pt3.x -= rc_.left;
				pt3.y -= rc_.top;

				int j = 0, k = float_pos_;
				for( auto& it : items_ )
				{
					if ( it.rc.PtInRect(pt3) )
						k = j;
					j++;
				}


				if ( k != float_pos_ )
				{
					float_pos_ = k;
					d->redraw();
				}

				ret = 1;
			}
			/*else
			{
				if ( pt3.x < rc_.left || rc_.right < pt3.x )
				{

					parent_control_->ReleaseCapture();
					Hide();
					ret = 1;
				}
			}*/

		}
		break;
		case WM_LBUTTONDOWN:
		{
			ret = 1;
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if (IsCaptured())
			{
				parent_control_->ReleaseCapture(this);

				Hide();
				ret = 1;

				WParameter ws;
				ws.target = target_;
				ws.no = items_[float_pos_].menuid;
				ws.sender = this;
				

				if ( target_ )
					target_->WndProc(parent_, WM_D2D_COMMAND, (INT_PTR)&ws, nullptr );
				else
					d->SendMessage( WM_D2D_COMMAND, (INT_PTR)&ws, nullptr );

				DestroyControl();
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Escape:
				{	
					if ( IsCaptured())
					{
						parent_control_->ReleaseCapture();
						Hide();
						ret = 1;	
						
						DestroyControl();
					}
				}
				break;
			}
		}
		break;
		case WM_D2D_MENU_ITEM_INSERT:
		{
			WParameter* ws = (WParameter*)wp;
			
			if ( ws->target == this )
			{
				BSTR json = (BSTR)ws->prm;
				target_ = ws->sender;

				ParseMenu(json);

				ret = 1;

				SysFreeString(json);
			}
		}
		break;
	}
	return ret;
}
void D2DVerticalMenu::ParseMenu(BSTR json)
{
	D2DMenuItem* h;
	int cnt;

	items_.clear();
	MenuItemsJsonParse( json, &h, &cnt );
	
	FRectF rcitem( 0,0,rc_.Width(), 30 );
	for( int i = 0; i < cnt; i++ )
	{
		Item item1;
		item1.name = h[i].name;
		item1.id = i;
		item1.rc = rcitem;
		item1.menuid = h[i].id;

		items_.push_back( item1 );
		rcitem.Offset( 0, rcitem.Height());
	}
				

	float cx = rc_.Width();
	float cy = rcitem.Height()*cnt;
	rc_.SetSize(cx,cy);

	MenuItemsClose( h, cnt );
	float_pos_ = -1;
}

#define XRC(xrc)  (FRectF(xrc.left+5, xrc.top+5, xrc.right, xrc.bottom ))

void D2DVerticalMenu::DrawItem( D2DContext& cxt, Item& it )
{
	if ( float_pos_ == it.id )
		cxt.cxt->FillRectangle( it.rc, cxt.halftoneRed );

	cxt.cxt->DrawText( it.name.c_str(), it.name.length(), cxt.textformat, XRC(it.rc), cxt.black );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DStatic::D2DStatic():fore_(ColorF::Black),text_(nullptr)
{

}
D2DStatic::~D2DStatic()
{
	Clear();
}
void D2DStatic::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	SetText( name, 0 );
}
int D2DStatic::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( !(stat_ & VISIBLE) )
		return 0;
		
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			FRectF rcb = rc_.GetBorderRect();
			mat.Offset(rcb.left, rcb.top); // border rect基準

			
			ComPTR<ID2D1SolidColorBrush> forebr;
			cxt.cxt->CreateSolidColorBrush( fore_, &forebr );
			
			//FRectF rc = rcb.ZeroRect();
						
			FRectF rca = rc_.GetContentBorderBase(); 

			D2D1_DRAW_TEXT_OPTIONS opt = D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP;
				
			cxt.cxt->DrawTextLayout( rca.LeftTop(), layout_, forebr, opt );

			mat.PopTransform();
		}
		break; 
	}

	return 0;
}



void D2DStatic::SetText( LPCWSTR txt, int align_typ, IDWriteTextFormat* tf )
{
	Clear();

	auto tf2 = parent_->cxt()->cxtt.textformat.p;
	auto wf = parent_->cxt()->cxtt.wfactory;

	auto sz = _rc().Size();

	auto tf3 = (tf == nullptr ? tf2 : tf );

	auto old = tf3->GetTextAlignment();

	D2DTextbox::s_SetAlign(tf3, align_typ );

	wf->CreateTextLayout( txt, wcslen(txt), tf3, sz.width, sz.height, &layout_ );

	tf3->SetTextAlignment(old);
		
	text_ = ::SysAllocString(txt);
}
void D2DStatic::SetFont( const FontInfo& cf, int typ )
{	
	auto wf = parent_->cxt()->cxtt.wfactory;
	auto tf = cf.CreateFormat( wf );
	
	std::wstring tex = text_;
	SetText( tex.c_str(),typ, tf);
}

void D2DStatic::Clear()
{
	layout_.Release();
	::SysFreeString( text_ );
	text_ = nullptr;
}
FRectF D2DStatic::_rc() const
{
	return rc_.GetContentRect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FSizeF V4::CreateTextLayout( D2DContext& cxt, LPCWSTR str, UINT strlen, IDWriteTextLayout** ret )
{
	FSizeF sz;
	auto wf = cxt.wfactory;
	if ( S_OK == wf->CreateTextLayout( str, strlen, cxt.cxtt.textformat, 1000,100, ret ) )
	{
		DWRITE_TEXT_METRICS tm;
		if ( S_OK == (*ret)->GetMetrics(&tm))
		{
			sz.height = tm.height;
			sz.width = tm.width;
		}
	}
	return sz;
}
ComPTR<ID2D1SolidColorBrush> V4::CreateBrush( D2DContext& cxt, D2D1_COLOR_F clr )
{
	ComPTR<ID2D1SolidColorBrush> br1;
	cxt.cxt->CreateSolidColorBrush( clr, &br1);
	return br1;
}