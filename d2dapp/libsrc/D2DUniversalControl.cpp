#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTextbox.h"

using namespace V4;
using namespace concurrency;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DButton::DefaultDrawButton( D2DButton* sender, D2DContext& cxt )
{
	FRectFBoxModel rc = sender->rc_.GetZeroRect();

	if (sender->stat_ & BORDER)
	{
		auto rcb = rc.GetBorderRect();
		cxt.cxt->FillRectangle(rcb, cxt.black);
	}
	
	ID2D1LinearGradientBrush* br = cxt.silver_grd;
	
	br->SetStartPoint( FPointF(0,rc.top));
	br->SetEndPoint( FPointF(0,rc.bottom));

	ComPTR<ID2D1SolidColorBrush> br2;
	cxt.cxt->CreateSolidColorBrush(D2RGBA(230,230,230,200 ), &br2 );
	
	FillRectangle( cxt, rc, br2 );

	FRectF rc2(rc);
	rc2.InflateRect( -2,-2);

	auto txtclr = cxt.black;

	if (sender->stat_ & STAT::MOUSEMOVE)
	{
		auto br = cxt.bluegray;
		FillRectangle(cxt, rc2, br);
		txtclr = cxt.white;

	}
	else if ( 0 < (sender->stat_ & STAT::FOCUS))
	{
		auto br = cxt.bluegray;
		//FillRectangle(cxt, rc2, br);

		auto rc = rc2.InflateRect(-3, -3);
		cxt.cxt->DrawRectangle( rc, br, 1,cxt.dot2_ );

		txtclr = cxt.black;
	}
	else
	{
		cxt.cxt->FillRectangle(rc2, br);		
	}

	cxt.cxt->DrawTextLayout(sender->layout_pt_, sender->textlayout_, cxt.black );
}

void D2DButton::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int controlid)
{
	InnerCreateWindow(pacontrol,rc,stat,name, controlid);
	mode_ = 0;
	title_ = title;
	SetText(title);
	OnPaint_ = DefaultDrawButton;
}

int D2DButton::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() && !IsImportantMsg(message))
		return 0;


	int ret = 0;

	switch( message )
	{
		case WM_PAINT:
		{			
			CXTM(d)

			mat_ = mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);


			OnPaint_( this, cxt );


			mat.PopTransform();
		}
		break; 
		case WM_MOUSEMOVE:
		{
			LOGPT(pt3, wp)
			auto stat = stat_;

			if (rc_.PtInRect(pt3))
				stat |= STAT::MOUSEMOVE;
			else
				stat &= ~STAT::MOUSEMOVE;


			if ( stat!=stat_)
			{
				stat_ = stat;
				d->redraw();
			}
		}
		break;
		case WM_D2D_NCHITTEST:
		{
			LOGPT(pt3,wp)
			
			if (rc_.PtInRect(pt3))
			{
				ret = HTCLIENT;
			}
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp)

			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
		}
		break;
		case WM_D2D_SETFOCUS:
		{
			D2DControl* pdc = (D2DControl*)wp;
			_ASSERT( pdc == this );
			{
				stat_ |= STAT::FOCUS;
				ret = 1;
				d->redraw();
			}
		}
		break;
		case WM_D2D_KILLFOCUS:
		{
			if (stat_ & STAT::FOCUS )
			{
				d->redraw();
				stat_ &= ~STAT::FOCUS;
				ret = 1;
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF ptd(lp);

			LOGPT(pt3, wp)

			mode_ = 0;

			if ( rc_.PtInRect(pt3 ))
			{
				d->SetFocus( this );

				mode_ = 1;
				parent_control_->SetCapture( this );
				ret = 1;
		
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
				
				LOGPT(pt3, wp)
				
				if ( rc_.PtInRect(pt3))
				{
					if ( OnClick_ )
						OnClick_( this );
				}
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;


			switch(arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:
				{
					if (parent_control_->GetCapture() == this)
					{
						parent_control_->ReleaseCapture();
						mode_ = 0;
					}
				}
				break;
				case Windows::System::VirtualKey::Enter:
				{
					if (OnClick_ && this == GetParentWindow()->GetFocus())
					{
						OnClick_(this);
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
void D2DButton::Clear()
{
	textlayout_.Release();

}

void D2DButton::SetText(LPCWSTR txt )
{
	title_ = txt; 
	Clear();

	auto tf2 = parent_->cxt()->cxtt.textformat;
	auto wf = parent_->cxt()->cxtt.wfactory;

	auto sz = rc_.GetContentRect().Size();

	auto hr = wf->CreateTextLayout(txt, (UINT32)wcslen(txt), tf2, 3000, 1000, &textlayout_);
	

	DWRITE_TEXT_METRICS mtrcs;
	textlayout_->GetMetrics(&mtrcs);

	FPointF pt((rc_.Width() - mtrcs.width) / 2.0f, (rc_.Height() - mtrcs.height) / 2.0f);

	layout_pt_ = pt;
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
	if (IsHide() && !IsImportantMsg(message))
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

	cxt.cxt->DrawText( it.name.c_str(), (UINT32)it.name.length(), cxt.textformat, XRC(it.rc), cxt.black );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DStatic::D2DStatic():fore_(ColorF::Black)
{

}
D2DStatic::~D2DStatic()
{
	Clear();
}
void D2DStatic::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	SetText( name, 0 );
}
int D2DStatic::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() && !IsImportantMsg(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{			
			CXTM(d)

			mat_ = mat.PushTransform();

			FRectF rcb = rc_.GetBorderRect();
			mat.Offset(rcb.left, rcb.top); // border rect基準

			
			ComPTR<ID2D1SolidColorBrush> forebr;
			cxt.cxt->CreateSolidColorBrush( fore_, &forebr );
					
			FRectF rca = rc_.GetContentBorderBase(); 
				
			cxt.cxt->DrawTextLayout( rca.LeftTop(), layout_, forebr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);

			mat.PopTransform();
		}
		break; 
	}

	return 0;
}



void D2DStatic::SetText( LPCWSTR txt, int align_typ, IDWriteTextFormat* tf )
{
	Clear();

	_ASSERT(align_typ==0 || align_typ == 1 || align_typ == 2);

	auto tf2 = parent_->cxt()->cxtt.textformat.p;
	auto wf = parent_->cxt()->cxtt.wfactory;

	auto sz = _rc().Size();

	auto tf3 = (tf == nullptr ? tf2 : tf );

	auto old = tf3->GetTextAlignment();

	D2DTextbox::s_SetAlign(tf3, align_typ );

	auto hr = wf->CreateTextLayout( txt, (UINT32)wcslen(txt), tf3, 3000, 1000, &layout_ );

	tf3->SetTextAlignment(old);
		
}

void D2DStatic::Clear()
{
	layout_.Release();
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
FSizeF V4::DrawFramelikeMFC(D2DContext& cxt, FRectF rc1, ID2D1Brush* backcolor)
{
	cxt.cxt->FillRectangle(rc1, cxt.frame[0]); rc1.Inflate(-3, -3);
	cxt.cxt->FillRectangle(rc1, cxt.frame[1]); rc1.Inflate(-1, -1);
	cxt.cxt->FillRectangle(rc1, cxt.frame[2]); rc1.Inflate(-1, -1);
	cxt.cxt->FillRectangle(rc1, backcolor);

	return FSizeF(5, 5);
}

bool V4::CreateLightThread(LPTHREAD_START_ROUTINE th, LPVOID prm, DWORD* thread_id)
{
	return ( nullptr != ::CreateThread(0, 0, th, prm, 0, thread_id));
}

DWORD V4::CreateLightThread(LPTHREAD_START_ROUTINE th, LPVOID prm)
{
	DWORD thread_id=0;
	::CreateThread(0, 0, th, prm, 0, &thread_id);
	return thread_id;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TITLE_BAR_HEIGHT 30

void D2DMessageBox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow(pacontrol, rc, stat, name, controlid);

	FRectF rc1(0,0, rc.Size());
	rc1.top = rc1.bottom- TITLE_BAR_HEIGHT;

	FRectF rcb(0,0,50,26);
	rcb = rcb.CenterRect(rc1);
	D2DButton* btnCancel = new D2DButton();
	btnCancel->Create(this, rcb, VISIBLE, _u("cancel"), NONAME, 1);

	btnCancel->OnClick_=[this](D2DButton* b)
	{
		Close();	
	};
}
void D2DMessageBox::Show(D2DControls* parent, FRectF rc, LPCWSTR title, LPCWSTR msg, int typ )
{
	auto controls = dynamic_cast<D2DControls*>(parent);
	auto win = controls->GetParentWindow();
	auto tf2 = win->cxt()->cxtt.textformat;
	auto wf = win->cxt()->cxtt.wfactory;

	if( rc.right-rc.left <= 0 || rc.bottom - rc.top <= 0 )
	{		
		FRectF rcw = win->GetClientRect();		
		rc = rcw.CenterRect(FSizeF(300,200));
		auto rc1 = controls->GetRect();
		rc.Offset( -rc1.left, -rc1.top );
	}
	
	
	D2DMessageBox* p = new D2DMessageBox();
	p->Create(controls, rc, VISIBLE, NONAME, -1);

	controls->SetCapture(p);

	p->result_ = IDCANCEL;
	

	wf->CreateTextLayout(msg, (UINT32)wcslen(msg), tf2, 1000, 1000, &p->msg_);
	wf->CreateTextLayout(title, (UINT32)wcslen(title), tf2, 1000, 1000, &p->title_);

}

void D2DMessageBox::Close()
{
	if (parent_control_)
	{
		stat_ = 0;
		auto p = parent_control_->ReleaseCapture(this); 

		_ASSERT(p == parent_control_);// captureはparent_control_に戻る

		DestroyControl();

		parent_control_ = nullptr;
		GetParentWindow()->SetFocus(nullptr);
	}
}
int D2DMessageBox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0; 

	if (IsHide() && !IsImportantMsg(message))
		return 0;

	switch (message)
	{
		case WM_PAINT:
		{
			CXTM(d)
				
			mat_ = mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);
				
			FRectF rcc = rc_.ZeroRect();

			cxt.cxt->DrawRectangle(rcc, cxt.black);
			cxt.cxt->FillRectangle(rcc, cxt.ltgray);


			DWRITE_TEXT_METRICS tm;

			// draw title
			title_->GetMetrics(&tm);
			FSizeF sz(tm.width, tm.height);
			FRectF rca( FPointF(0, 0), sz );
			FRectF rc1(rcc);
			rc1.SetHeight(TITLE_BAR_HEIGHT);
			FRectF rcb = rca.CenterRect(rc1);
			cxt.cxt->FillRectangle(rc1, cxt.bluegray);
			cxt.cxt->DrawTextLayout( rcb.LeftTop(), title_, cxt.white);

			// draw message
			msg_->GetMetrics(&tm);
			FSizeF sz1(tm.width, tm.height);
			rca.SetRect(FPointF(0, 0), sz1);
			rca = rca.CenterRect(rcc);
			cxt.cxt->DrawTextLayout(rca.LeftTop(), msg_, cxt.black);



			DefPaintWndProc(d, message, wp, lp);

			mat.PopTransform();			
			return 0;
		}

		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt(lp);

			FPointF pt3 = mat_.DPtoLP(pt);

			if (rc_.PtInRect(pt3))
			{
				//Close();
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			switch (arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:
				{
					Close();
					ret = 0;
				}
				break;
			}

		}
		break;

	}
	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);
	return ret;
}

////////////////////////////////////
void V4::MoveMatrixY(D2DWindow* d, float* pouty, float offy, int step, int step_msec)
{
	float sy = *pouty;
	float ey = sy + offy;

	create_async([d, sy, ey, pouty, step,step_msec]() {
		
		float base = (ey - sy) / step;

		float* py = new float[step];

		for (int i = 0; i < step; i++)
		{
			py[i] = sy + base * i;
		}

		for (int i = 0; i < step; i++)
		{
			*pouty = py[i];

			d->redraw();

			Sleep(step_msec);
		}

		*pouty = ey;
		d->redraw();
		delete [] py;

	});
}
void V4::MoveD2DControl(D2DWindow* d, D2DControl* pc, float offx, float offy, int step, int step_msec)
{
	FRectF rc = pc->GetRect();

	float sx = rc.left;
	float sy = rc.top;

	float ex = rc.left + offx;
	float ey = rc.top + offy;

	d->SendMessage(WM_D2D_UI_LOCK,0,nullptr);


	create_task(create_async([d,rc,sx,ex, sy, ey, pc, step, step_msec]() {

		float basex = (ex - sx) / step;
		float basey = (ey - sy) / step;

		FRectF* rcc = new FRectF[step];

		auto sz = rc.GetSize();
		for (int i = 0; i < step; i++)
		{
			auto x = sx + basex * i;
			auto y = sy + basey * i;

			rcc[i].SetRect( x,y,sz);
		}

		for (int i = 0; i < step; i++)
		{
			pc->SetRect( rcc[i] );

			d->redraw();

			Sleep(step_msec);
		}

		delete[] rcc;

	})).then([d](){
	
		d->SendMessage(WM_D2D_UI_UNLOCK, 0, nullptr);
		
		D2DControls* a = dynamic_cast<D2DControls*>(d);
		a->ReleaseCapture();
	
	});;
}