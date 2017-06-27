#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDriftDialog.h"

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
	// ループ内で実行する場合は、必ず１を返すこと
	if ( !(stat_ & STAT::DEAD ) )
	{		
		stat_ &= ~STAT::VISIBLE;
		stat_ |= STAT::DEAD;

		if ( parent_control_->GetCapture() == this )
			parent_control_->ReleaseCapture();


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
	if ( ParentControl()->GetCapture() == this )
		ParentControl()->ReleaseCapture();
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
void D2DControls::SetCapture(D2DCaptureObject* p, int layer )
{
	_ASSERT( p != nullptr );
	
	if ( layer == 0 )
	{
		_ASSERT ( parent_ != nullptr );

		if ( dynamic_cast<D2DMainWindow*>(parent_)->GetTopCapture() == p )
			return;


		ReleaseCapture(0); // all objects are released.

		dynamic_cast<D2DMainWindow*>(parent_)->SetTopCapture(p);
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
	_ASSERT( layer == 0 || layer == -1 );

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

void D2DButton::CreateButton(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int controlid)
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

				auto x = ParentWindow();

				
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
void D2DMessageBox::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);
	
}

void D2DMessageBox::Show(D2DWindow* pa, D2DControls* pacontrol,FRectF rc,  LPCWSTR title, LPCWSTR msg )
{
	D2DMessageBox* p = new D2DMessageBox();

	auto x = pa;

	rc.SetSize(300,200);

	p->Create( x, pacontrol, rc, VISIBLE, NONAME,-1 );

	pacontrol->SetCapture( p );

	p->result_ = IDCANCEL;
	p->msg_ = msg;
	p->title_ = title;
}
int D2DMessageBox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( this->IsHide())
		return 0;


	int ret =  ( parent_control_->GetCapture() == this ? 1 : 0);

	switch ( message )
	{
		case WM_PAINT:
		{
		
			{
				auto& cxt = *(d->cxt());

				D2DMatrix mat(cxt);
				mat_ = mat.PushTransform();
				//mat.Offset(rc_.left, rc_.top);


				FRectF rc = rc_;
				
				cxt.cxt->DrawRectangle( rc, cxt.black );
				cxt.cxt->FillRectangle( rc, cxt.ltgray );

				FRectF rca(0,0, 200, 30);

				rca.CenterRect( rc_ );

				cxt.cxt->DrawText(title_.c_str(), title_.length(), cxt.cxtt.textformat, rca, cxt.black );

				rca.Offset( 0, 40 );
				cxt.cxt->DrawText(msg_.c_str(), msg_.length(), cxt.cxtt.textformat, rca, cxt.black );
				


				mat.PopTransform();

			}
			return 0;
		}

		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt(lp);
						
			FPointF pt3 = mat_.DPtoLP(pt);

			if ( rc_.PtInRect(pt3))
			{
				stat_ &= ~VISIBLE;
				

				parent_control_->ReleaseCapture();

				DestroyControl();

				ret = 1;
			}
		}
		break;		
		case WM_KEYDOWN :
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			switch( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Escape:
				{
					parent_control_->ReleaseCapture(-1);
					DestroyControl();
					ret = 1; 
				}
				break;
				case Windows::System::VirtualKey::T:
				{
					FRectF rc( 0,0, FSizeF(300,300));

					D2DMessageBox::Show( parent_, parent_control_, rc, L"test", L"this is test" );
					ret = 1;
				}
				break;
			}

		}
		break;
		
	}

	return ret;
}

////////////////////////////////////
FontInfo::FontInfo() //:forecolor(ColorF::Black),backcolor(ColorF::White)
{
	height = 12;
	fontname = L"メイリオ";
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

void D2DGroupControls::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	_ASSERT( rc.left == 0 );
	_ASSERT( rc.top == 0 );
	
	
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);
	drift_ = nullptr;
	drift_typ_ = 0;
	drift_value_ = 0;


	FRectF menurc(0,0,rc.Width(), 20 );
	D2DTitlebarMenu* menu = new D2DTitlebarMenu(); // topのタイトル下のメニュー
	menu->Create( parent, this, menurc, 0, L"noname" );

	menu_ = menu;

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
						menu_->Visible();
					else
						menu_->Hide();


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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DTitlebarMenu::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);

	floating_idx_ = -1;
	for( int i = 0; i < 4; i++ )
	{
		items_.push_back( FRectF(0,0,100,20 ));

		D2DVerticalMenu* x = new D2DVerticalMenu();
		
		FRectF rc(0,0,600,300);
		x->Create( parent,this, rc, 0, L"noname" );		

	}
	
}
int D2DTitlebarMenu::WndProc(D2DWindow* d, int message, int wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
				
					if ( nullptr == GetCapture() )
						SetCapture(t.get());

					ret = 1;
				}
			}
			else if ( GetCapture() )
			{
				//ret = HideMenu();
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
int D2DVerticalMenu::WndProc(D2DWindow* d, int message, int wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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


			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt3))
			{
				parent_control_->ReleaseCapture();
				Hide();
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
					parent_control_->ReleaseCapture();
					Hide();
					ret = 1;					
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


