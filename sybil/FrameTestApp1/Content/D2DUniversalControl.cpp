#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDriftDialog.h"
#include "D2DSplitControls.h"
using namespace V4;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	target_ = nullptr; // このオブジェクトと関連付けるポインタ、通常はnull

	if (parent_control_)
	{
		auto target = std::shared_ptr<D2DControl>(this);
		parent_control_->controls_.push_back( target ); 
	}

	auto mainparent = dynamic_cast<D2DMainWindow*>(parent);
	if ( mainparent )
	{
		if ( name_ != L"noname" )
			mainparent->hub_[ name_ ] = this;
	}	
}
void D2DControl::DestroyControl()
{
	_ASSERT( parent_control_->GetCapture() != this );

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
		GetParentControl()->ReleaseCapture();
}
void D2DControl::OnSetCapture(int layer)
{
	stat_ |= STAT::CAPTURED;
}
void D2DControl::OnReleaseCapture(int layer)
{
	stat_ &= ~STAT::CAPTURED;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int D2DControls::WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	// 直接、DefWndProcをよべ
	return 0;
}

int D2DControls::DefPaintWndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	_ASSERT( message == WM_PAINT );

	int ret = 0;
		
	if ( !capture_.empty() )
	{
		// paintは逆順でループ
		for( auto it = controls_.rbegin(); it != controls_.rend(); it++ )
		{
			if ( (*it)->IsCaptured() == false )
				ret = (*it)->WndProc(d,message,0,lp);
		}

		// captureされたものを最後に表示
		if ( !capture_.empty() )
			(*capture_.ar_.begin())->WndProc(d,message,1,lp);  // 1: paint in capturing		
	
		return ret;
	}
	else
	{
		// paintは逆順でループ
		for( auto it = controls_.rbegin(); it != controls_.rend(); it++ )
		{
			if ( 0 != ( ret= (*it)->WndProc(d,message,0,lp)) )
				break;
		}
	}
	return ret;
}

int D2DControls::DefWndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	_ASSERT( message != WM_PAINT );

	int ret = 0;

	if ( !capture_.empty() )
	{		
		auto cap = capture_.top();

		auto pr = cap;
			
		ret = cap->WndProc(d,message,wp,lp); // capture_.empty()になることがある

		if ( ret == 0 ) //|| capture_.empty() )
		{
			for( auto& it : controls_ )
			{
				if ( it.get() != pr )
					ret = it->WndProc(d,message,wp,lp);

				if ( ret ) break;
			}
		}
	}
	else
	{		
		for( auto& it : controls_ )		
		{
			if ( 0 != ( ret= it->WndProc(d,message,wp,lp)) )
				break;
		}
	}

	return ret;
}
std::shared_ptr<D2DControl> D2DControls::Detach(D2DControl* target)
{
	for( auto it = controls_.begin(); it != controls_.end(); ++it )
	{
		if ( (*it).get() == target )
		{
			std::shared_ptr<D2DControl> ret = (*it);

			controls_.erase( it );

			ret->parent_control_ = nullptr;

			return ret;
		}
	}

	_ASSERT( false );

	return nullptr;
}


void D2DControl::SetCapuredLock(bool lock )
{
	// この機能は動作しない？

	if ( lock )
		stat_ |= STAT::CAPTURED_LOCK;
	else
		stat_ &= ~STAT::CAPTURED_LOCK;
}
void D2DControls::SetPrevCapture(D2DCaptureObject* p)
{
	if ( p )
	{
		auto t = dynamic_cast<D2DControl*>(p);
		t->SetCapuredLock(true);
	}
}	


void D2DControls::SetCaptureByChild(D2DCaptureObject* p, int layer )
{
	SetCapture(p,layer);

	SetPrevCapture(this);

}
void D2DControls::SetCapture(D2DCaptureObject* p, int layer )
{
	_ASSERT( p != nullptr );
	
	if ( layer == 0 )
	{
		_ASSERT ( parent_ != nullptr );
		_ASSERT ( dynamic_cast<D2DControl*>(p)->GetParentControl() == this );


		if ( dynamic_cast<D2DMainWindow*>(parent_)->GetTopCapture() == p )
			return;

		SetPrevCapture( dynamic_cast<D2DMainWindow*>(parent_)->GetTopCapture());


		ReleaseCapture(0); // all objects are released.

		dynamic_cast<D2DMainWindow*>(parent_)->SetTopCapture(p);
	}

	// Capture obj move to top.
	if (controls_.size() > 1)
	{
		auto it = controls_.begin();
		for( ; it != controls_.end(); it++ )
		{
			if ( (*it).get() == p )
			{
				auto obj = (*it);
				controls_.erase(it);
				controls_.insert( controls_.begin(), obj );
				break;
			}
		}
		
	}



	// 自分(D2DControls)もcapture対象！
	if ( parent_control_  )
		parent_control_->SetCapture(this, layer+1);
	
	// キャプチャー
	capture_.push(p);

	p->OnSetCapture(layer);

}
D2DCaptureObject* D2DControls::ReleaseCapture( int layer )
{
	_ASSERT( layer == 0 || layer == -1  );

	if ( parent_ == nullptr ) return nullptr;

	auto x = dynamic_cast<D2DMainWindow*>(parent_)->SetTopCapture(nullptr);

	if ( x )
	{
		D2DControls* xpa = dynamic_cast<D2DControl*>(x)->parent_control_;
		xpa->ReleaseCaptureEx(layer);
	}

	return x;
}

D2DCaptureObject* D2DControls::ReleaseCaptureEx( int layer )
{
	if( !capture_.empty() )
	{
		auto r = capture_.top();
		capture_.pop();

				
		if ( layer == -1 )
		{
			if ( parent_ )
				dynamic_cast<D2DMainWindow*>(parent_)->SetTopCapture(this);
		}
		else if ( parent_control_  )
			parent_control_->ReleaseCaptureEx( layer+1 );

		r->OnReleaseCapture(layer);

		
		return r;
	}
	return nullptr;
}
D2DCaptureObject* D2DControls::GetCapture()
{
	return (capture_.empty() ? nullptr : capture_.top());
}

void D2DControls::OnDXDeviceLost() 
{ 
	for( auto& it : controls_ ) it->OnDXDeviceLost(); 
}
void D2DControls::OnDXDeviceRestored()  
{ 
	for( auto& it : controls_ ) it->OnDXDeviceRestored(); 
}

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

void D2DButton::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int controlid)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);
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
				parent_control_->ReleaseCapture(-1);
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

FontInfo::FontInfo() //:forecolor(ColorF::Black),backcolor(ColorF::White)
{
	height = 12;
	fontname = DEFAULTFONT;
	weight = 400;
}

ComPTR<IDWriteTextFormat> FontInfo::CreateFormat( IDWriteFactory* wfac ) const
{
	_ASSERT( wfac );

	ComPTR<IDWriteTextFormat> fmt;
	
	wfac->CreateTextFormat(
		fontname.c_str(),
		nullptr,
		(DWRITE_FONT_WEIGHT)weight,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		height,
		DEFAULTLOCALE,
		&fmt
	);

	return fmt;
}
		



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DTitlebarMenu::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);

	floating_idx_ = -1;
	for( int i = 0; i < 1; i++ )
	{
		items_.push_back( FRectF(0,0,100,20 ));

		D2DVerticalMenu* x = new D2DVerticalMenu();
		
		FRectF rc(0,0,200,300);
		x->Create( parent,this, rc, 0, L"noname" );		

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



			cxt.cxt->FillRectangle( rc_.ZeroRect(), DRGB( D2RGB(200,200,200)));

			{
				int j = 0;
				FRectF rc(0,0,0,0);
				for( auto& it  : items_ )
				{
					rc.SetSize( it.Size());
					cxt.cxt->DrawText( L"item", 4, cxt.textformat, rc, cxt.black );
					if ( j++ == floating_idx_ )
						cxt.cxt->FillRectangle( rc, DRGB( D2RGBA(0,255,0,100) ));

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
			if ( 0 <= selected_idx_ && selected_idx_ < controls_.size())
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
int D2DVerticalMenu::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
						
			cxt.cxt->DrawRectangle( rc_.ZeroRect(), cxt.black );
			cxt.cxt->FillRectangle( rc_.ZeroRect(), cxt.white );

			for( auto& it : items_ )
			{
				Draw(cxt, it);
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
			else
			{
				if ( pt3.x < rc_.left || rc_.right < pt3.x )
				{

					parent_control_->ReleaseCapture();
					Hide();
					ret = 1;
				}
			}

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
			//if ( rc_.PtInRect(pt3))
			
			if ( IsCaptured())
			{
				parent_control_->ReleaseCapture();
				Hide();
				ret = 1;

				d->SendMessage( WM_D2D_COMMAND, items_[float_pos_].menuid, nullptr );

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
					}
				}
				break;
			}
		}
		break;
	}
	return ret;
}
void D2DVerticalMenu::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);

	LPCWSTR json = L"[{\"name\":\"hoi\", \"id\":1}, {\"name\":\"hoi2\", \"id\":2}, {\"name\":\"close\", \"id\":9999}]]";

	D2DMenuItem* h;
	int c;

	MenuItemsJsonParse( json, &h, &c );
	
	FRectF rcitem( 0,0,rc_.Width(), 30 );
	for( int i = 0; i < c; i++ )
	{
		Item item1;
		item1.name = h[i].name;
		item1.id = i;
		item1.rc = rcitem;
		item1.menuid = h[i].id;

		items_.push_back( item1 );
		rcitem.Offset( 0, rcitem.Height());

	}
		
	MenuItemsClose( h, c );

	float_pos_ = -1;

}

#define XRC(xrc)  (FRectF(xrc.left+5, xrc.top+5, xrc.right, xrc.bottom ))

void D2DVerticalMenu::Draw( D2DContext& cxt, Item& it )
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
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);

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


