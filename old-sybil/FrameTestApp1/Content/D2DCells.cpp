#include "pch.h"
#include "D2DWindowMessage.h"
#include "D2DCells.h"
#include "D2DTextbox.h"
#include "higgsjson.h"


using namespace V4;
using namespace HiggsJson;


#define INIT_CELL_WIDTH			76.0f
#define INIT_CELL_HEIGHT		25.0f
#define CONTROLBAR_HEIGHT		50.0f
#define TEXTBOX_TRIMMING_LEFT	4.0f
#define TEXTBOX_TRIMMING_TOP	1.0f

#define ROW_LABEL_W			28.0f
#define COL_LABEL_H			INIT_CELL_HEIGHT


static LPCWSTR ColumnABC(int* len, int val);
static LPCWSTR RowNumber(int* len, int val);

#define BARWIDTH 18
#define SCBAR(x) ((D2DScrollbar*)x.get())

static void BackGroundWhite(D2DContext& cxt, D2D1_RECT_F& rc )
{
	cxt.cxt->FillRectangle(rc, cxt.white);
}



int D2DCells::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() && !IsImportantMsg(message)) 
		return 0;

	int ret = 0;
		
	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = *(d->cxt());

			D2DMatrix mat(cxt);	
			mat_ = mat.PushTransform();
			mat.PushTransform();
			{
				//mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
				Draw( cxt, wp,lp );
			}
			mat.PopTransform();
			
			Vscbar_->WndProc(d,message,wp,lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);

			if (tbox_->IsVisible())
			{
				ret = 0; // send message to textbox.
			}
			else if ( rc_.PtInRect(pt3))
			{
				pt3 = matd_.DPtoLP(lp);
				auto rc = RCPos(pt3);

				if ( rc.x == sel_.x && rc.y == sel_.y )
					ActiveTextbox(); // show and enable textbox in selsected cell
				else
					SetSelectCell(rc); // new selected cell

				ret = 1;
			}
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt3))
			{
				pt3 = matd_.DPtoLP(lp);
				SetSelectCell(RCPos(pt3));

				ActiveTextbox();
				
				ret = 1;
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect( pt3 ))		
			{
				ret = InnerDefWndScrollbarProc(d,message,wp,lp);
			}
		}
		break;

		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		case WM_D2D_RESIZE:
		{
			FSizeF sz = GetParentControl()->GetRect().Size();

			rc_.SetSize(sz);

			
			auto xrc = VScrollbarRect(rc_.ZeroRect());
			Vscbar_->SetRect(xrc);

			
			if ( message == WM_D2D_RESIZE )
			{
				bar_->WndProc(d,message,wp,lp);
				return 0;
			}

			if ( message == WM_D2D_INIT_UPDATE ||  message == WM_SIZE)
				DefPaintWndProc(d,message,wp,lp);
			return 0;
		}
		break;
		case WM_KEYDOWN:
		{			
			if ( !tbox_->IsCaptured())
			{			
				Windows::UI::Core::KeyEventArgs ^args = (Windows::UI::Core::KeyEventArgs ^)lp;
				if ( args->VirtualKey == Windows::System::VirtualKey::Left )
				{
					sel_.x = max(0,sel_.x-1);
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Right )
				{
					sel_.x = min(col_limit()-1,sel_.x+1);
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Up )
				{
					sel_.y = max(0,sel_.y-1);
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Down )
				{
					sel_.y = min(row_limit()-1,sel_.y+1);
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Escape )
				{					
					ReleaseCapture();					
					tbox_->Hide();


					D2DContext& cxt = *(d->cxt());

					LPCWSTR cellnm = SelectCellName();
					auto value =  values_[sel_].str;

					Cell2 c;
					c.str = value;
					c.sz = CreateTextLayout(cxt, value.c_str(), value.length(), &c.txt);
					c.pt = CellPos(cellnm);		
					c.zrc = ZRCSelectPos();
					values_[sel_] = c;
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Enter)
				{
					if (tbox_->IsHide())
					{
						auto rc = sel_;
						rc.y = min(sel_.y+1, row_limit());
						SetSelectCell(rc);
					}
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Home)
				{					
					auto sel = RCSelectPos();

					sel.x = 0;
					sel.y = 0;

					SetSelectCell(sel);

				}
				ret = 1;

			}

		}
		break;
		case WM_D2D_TEXTBOX_CHANGED:
		{
			WParameter* wps = (WParameter*)wp;

			if ( wps->sender == tbox_ )
			{
				ReleaseCapture();					
				tbox_->Hide();
				
				LPCWSTR txt = (LPCWSTR)wps->prm;
				D2DContext& cxt = *(d->cxt());

				LPCWSTR cellnm = SelectCellName();
				std::wstring value = txt;

				Cell2 c;
				c.str = value;
				c.sz = CreateTextLayout(cxt, value.c_str(), value.length(), &c.txt);
				c.pt = CellPos(cellnm);	
				c.zrc = ZRCSelectPos();

				//auto zsel = sel_;
				
				values_[ZRCSelectPos()] = c;

				//auto rc = sel_;
				//rc.y = min(row_limit(), sel_.y+1);
				//SetSelectCell(rc);

				ret = 1;
			}
		}
		break;
		case WM_D2D_CELLS_CONTROLBAR_SHOW:
		{
			if (bar_->IsVisible())
				bar_->Hide();
			else
				bar_->Visible();

			
			WndProc(d,WM_D2D_RESIZE,0,nullptr);
			ret = 1;
		}
		break;

		case WM_D2D_ESCAPE_FROM_CAPTURED:
		{
			ReleaseCapture();					
			tbox_->Hide();
					
			ret = 1;	
		}		
		break;

	}

	if ( ret == 0 )
		ret = DefWndProc(d,message,wp,lp);


	return ret;
}

FRectF D2DCells::SelectCellF()
{
	FPointF pt = CellF(sel_);		 // (xpos_[sel_.x], ypos_[sel_.y]);
	return FRectF(pt.x, pt.y, FSizeF(unit_w_,unit_h_));
}
LPCWSTR D2DCells::SelectCellName()
{
	return CellName( ZRCSelectPos() );
}

int D2DCells::col_limit()
{
	return 26+26;
}
int D2DCells::row_limit()
{
	return 200;
}
D2DCells::RowCol D2DCells::RCPos( FPointF pt )
{
	RowCol rc;
	rc.x = max(0, (int)(pt.x / unit_w_));
	rc.y = max(0, (int)(pt.y / unit_h_));
	return rc;
}
D2DCells::RowCol D2DCells::ZRCSelectPos()
{
	RowCol rc = sel_;
	rc.y += vrows_;
	return rc;
}

void D2DCells::ActiveTextbox()
{
	auto it = values_.find(ZRCSelectPos());
	if ( it != values_.end())
		tbox_->SetText( it->second.str.c_str() );
	else
		tbox_->SetText(L"");
				
	FRectF rc = SelectCellF();
	rc.right = rc.left + 500;
	FRectFBoxModel rc1 = tbox_->GetRect();
	rc1.SetRect(rc.LeftTop(), rc.GetSize());
	tbox_->SetRect(rc1);

	ReleaseCapture();
	tbox_->Visible();
	tbox_->Activate();		
}


LPCWSTR D2DCells::CellName(RowCol rc)
{
	static WCHAR nm[8];
	
	int len;
	auto cb = ColumnABC(&len,rc.x);

	StringCbPrintf(nm,8,L"%s%d",cb, rc.y+1); 
	return nm;
}




void D2DCells::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, DT typ, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	FRectFBoxModel xrc = rc.GetContentRectZero();
	xrc.left = xrc.right - BARWIDTH;

	auto p = new D2DScrollbar();
	p->Create(this, xrc,VISIBLE,NONAME);
	Vscbar_ = controls_[0];
	controls_.clear();


	unit_w_ = INIT_CELL_WIDTH;
	unit_h_ = INIT_CELL_HEIGHT;
	//draw_mode_ = (DT)(DT::EXCELLINE | DT::EXCELTITLE | DT::NORMAL);
	draw_mode_ = typ;

	PreDraw();

	FRectFBoxModel txrc(0,0,unit_w_,unit_h_);
	txrc.Padding_.l = TEXTBOX_TRIMMING_LEFT;
	txrc.Padding_.t = TEXTBOX_TRIMMING_TOP;

	auto& caret = Caret::GetCaret();
	D2CoreTextBridge*  ime_bridge =  dynamic_cast<D2DMainWindow*>(pacontrol->GetParentWindow())->GetImeBridge();
	tbox_ = new D2DTextbox(*ime_bridge, caret);
	tbox_->Create(this, txrc, 0, NONAME);
	tbox_->SetBackground(BackGroundWhite);

	bar_ = new D2DCellsControlbar();
	bar_->Create(this,FRectF(0,0,rc_.Width(), CONTROLBAR_HEIGHT), 0, NONAME );


	// hide parent's vscrollbar
	WParameter wp;
	wp.sender = this;
	wp.target = pacontrol;
	wp.no = 0;
	pacontrol->WndProc(parent_, WM_D2D_VSCROLLBAR_SHOW, (INT_PTR)&wp,nullptr);

	md_ = MODE::NONE;


	// Set my vertical scrollbar.
	SetCanvasSize(0,INIT_CELL_HEIGHT*row_limit());
	auto& vinfo = SCBAR(Vscbar_)->Info();
	vinfo.row_height = INIT_CELL_HEIGHT;
}

void D2DCells::PreDraw()
{
	D2DContext& cxt = *(parent_->cxt());

	//
	// 上と左のタイトル文字設定
	//
	toptitle_.clear();
	lefttitle_.clear();

	int len;
	for(int i=0; i<col_limit(); i++)
	{		
		ComPTR<IDWriteTextLayout> txt;		
		auto cb = ColumnABC(&len, i);
		auto sz = CreateTextLayout(cxt, cb, len, &txt);
		Cell t;
		t.txt = txt;
		t.pt = FPointF( (unit_w_-sz.width)/2, (unit_h_-sz.height)/2);
		toptitle_.push_back(t);		
	}
	for(int i=0; i<row_limit(); i++)
	{		
		ComPTR<IDWriteTextLayout> txt;		
		auto cb = RowNumber(&len, i);
		auto sz = CreateTextLayout(cxt, cb, len, &txt);
		Cell t;
		t.txt = txt;
		t.pt = FPointF( (ROW_LABEL_W-sz.width)/2,(unit_h_-sz.height)/2);
		lefttitle_.push_back(t);
	}

	// 
	// タイトル表示位置の設定
	//
	int cnt = 1+ (int)(rc_.Width() / unit_w_);
	int rcnt = 1+ (int)(rc_.Height() / unit_h_);

	xpos_.clear();
	ypos_.clear();


	xpos_.push_back(0);
	ypos_.push_back(0);

	FRectF rcell(ROW_LABEL_W,0,FSizeF(unit_w_,unit_h_));
	auto rc1 = rcell;

	float xpos = unit_w_;

	for(int i =0; i< col_limit() ; i++)
	{		
		auto t = toptitle_[i];
		auto pt = t.pt;
		pt.x += rc1.LeftTop().x;

		toptitle_[i].pt = pt;

		xpos_.push_back(xpos);

		rc1.Offset(unit_w_,0);
		xpos += unit_w_;
	}

	rcell.SetRect(0,unit_h_,FSizeF(ROW_LABEL_W,unit_h_));
	rc1 = rcell;

	float ypos = COL_LABEL_H; // A,B,Cタイトル高さ

	for(int r=0; r< row_limit(); r++ )
	{			
		auto t = lefttitle_[r];
		auto pt = t.pt;
		pt.y += rc1.LeftTop().y;

		lefttitle_[r].pt = pt;
		
		ypos_.push_back(ypos);

		rc1.Offset(0,unit_h_);

		ypos += unit_h_;
	}


	//
	// sample
	//

	{
		LPCWSTR cellnm = L"A1";
		LPCWSTR value = L"ここのセルはA1です";

		Cell2 c;
		c.str = value;
		c.sz = CreateTextLayout(cxt, value, wcslen(value), &c.txt);
		c.pt = CellPos(cellnm);	

		RowCol rc = CellRC(cellnm);

		c.zrc = rc;
		values_[rc] = c;
	}

	{
		LPCWSTR cellnm = L"E7";
		LPCWSTR value = L"ここのセルはE7です";

		Cell2 c;
		c.str = value;
		c.sz = CreateTextLayout(cxt, value, wcslen(value), &c.txt);
		c.pt = CellPos(cellnm);	

		RowCol rc = CellRC(cellnm);

		c.zrc = rc;
		values_[rc] = c;
	}

	this->back_ground_ = BackGroundWhite;

}
D2DCells::RowCol D2DCells::CellRC(LPCWSTR cellnm)
{
	WCHAR col[3], row[8];
	int ix = 0, iy = 0;
	for(int i =0; i < (int)wcslen(cellnm); i++)
	{
		if ( '0' <= cellnm[i] && cellnm[i] <= '9' )
			row[iy++] = cellnm[i];	
		else if ( 'A' <= cellnm[i] && cellnm[i] <= 'Z' )
			col[ix++] = cellnm[i];	
	}
	col[ix]=0;
	row[iy]=0;

	if ( wcslen(col) == 1 )
		ix = col[0] - 'A';
	else if ( wcslen(col) == 2)
	{
		int k = col[0] - 'A' + 1;
		ix = k*26 + col[1] - 'A';
	}
	
	iy = _wtoi(row) - 1;
	
	RowCol rc;
	rc.x = ix;
	rc.y = iy; 
	return rc;
}

FPointF D2DCells::CellPos( LPCWSTR cellnm )
{
	auto rc = CellRC(cellnm);
	return FPointF(xpos_[rc.x], ypos_[rc.y]);
}
FPointF D2DCells::CellF(RowCol rc)
{
	return FPointF(xpos_[rc.x], ypos_[rc.y]);
}

void D2DCells::SetSelectCell(RowCol rc)
{
	sel_ = rc;

	if ( rc.y == 0 )
	{
		SCBAR(Vscbar_)->SetScrollbarTop();

	}

	WndProc(parent_,WM_D2D_CELLS_SELECT_CHANGED,0,nullptr);
}

void D2DCells::Draw(D2DContext& cxt, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp )
{
	int cnt = 1+ (int)(rc_.Width() / unit_w_);
	int rcnt = 1+ (int)(rc_.Height() / unit_h_);
	
	D2DMatrix mat(cxt);	

	mat.PushTransform();

	back_ground_(cxt,rc_);

	if ( bar_->IsVisible())
	{
		bar_->WndProc(parent_,WM_PAINT,wp,lp);
		
		mat.Offset(0,CONTROLBAR_HEIGHT); // 下段に下げる
	}


	const int start_row = scrollbar_off_.height/INIT_CELL_HEIGHT;
	vrows_ = start_row;	
	rcnt = rcnt + start_row;
	vrowe_ = min(row_limit(),rcnt);

	if (draw_mode_ & EXCELTITLE)
	{
		// draw label

		FRectF rcell(ROW_LABEL_W,0,FSizeF(unit_w_,unit_h_));
		auto rc1 = rcell;
		for(int i =0; i< min(col_limit(),cnt) ; i++)
		{		
			cxt.cxt->DrawRectangle( rc1, cxt.gray );

			auto br = (sel_.x == i ? cxt.gray : cxt.ltgray);
			cxt.cxt->FillRectangle( rc1, br );
						
			auto t = toptitle_[i];
			cxt.cxt->DrawTextLayout(t.pt, t.txt, cxt.black);
			rc1.Offset(unit_w_,0);
		}

		rcell.SetRect(0,unit_h_,FSizeF(ROW_LABEL_W,unit_h_));
		rc1 = rcell;
		
		
		float ypos = COL_LABEL_H;

		for(int r= vrows_; r <vrowe_; r++ )
		{			
			cxt.cxt->DrawRectangle( rc1, cxt.gray );

			auto br = (sel_.y == r ? cxt.gray : cxt.ltgray);
			cxt.cxt->FillRectangle( rc1, br );

			//auto t = lefttitle_[r];
			//cxt.cxt->DrawTextLayout(t.pt, t.txt, cxt.black);

			FRectF rc(5, ypos,FSizeF(200,30));
			WCHAR nos[100];
			StringCbPrintf(nos,100,L"%d", r+1);
			cxt.cxt->DrawText(nos,wcslen(nos),cxt.textformat,rc, cxt.black );

			ypos += unit_h_;

			rc1.Offset(0,unit_h_);
		}

		
		mat.Offset(ROW_LABEL_W, COL_LABEL_H );		
	}
	//
	// エクセル風セルとそのライン描画
	//

	if ( draw_mode_ &EXCELLINE )
	{
		FRectF rcell(ROW_LABEL_W,0,FSizeF(unit_w_,unit_h_));
		auto rc1 = rcell;

		for(int r=0; r<rcnt; r++)
		{
			auto rc = rc1;
			for(int i =0; i<cnt; i++)
			{		
				rc.left = xpos_[i];
				rc.right = xpos_[i+1];
								
				cxt.cxt->DrawRectangle( rc, cxt.gray );
				cxt.cxt->FillRectangle( rc, cxt.white );				
			}
			rc1.Offset(0,unit_h_);
		}
	}

	matd_ = mat;

	
	//
	// Content描画
	//
	if ( draw_mode_ &NORMAL )
	{
		for( auto& itk :values_ )
		{
			auto& it = itk.second;

			auto pt = it.pt;
			pt.x += TEXTBOX_TRIMMING_LEFT;
			pt.y += TEXTBOX_TRIMMING_TOP;
			pt.y -= vrows_*unit_h_;

			auto pt2 = it.pt;
			pt2.y -= vrows_*unit_h_;

			if ( vrows_  <= it.zrc.y )
			{
				cxt.cxt->FillRectangle( FRectF(pt2, it.sz), cxt.white);
				cxt.cxt->DrawTextLayout(pt, it.txt, cxt.black);
			}
		}

		// draw selected cell
		if (tbox_->IsHide())
		{
			FPointF pt = CellF(sel_); 

			//TRACE( L"%02f,%02f   rc=%d,%d\n", pt.x, pt.y, sel_.x, sel_.y);
			
			FRectF selrc(pt.x, pt.y, FSizeF(unit_w_,unit_h_));
		
			cxt.cxt->DrawRectangle(selrc, cxt.green);
		
			FRectF rc1(selrc.right-2,selrc.bottom-2,FSizeF(4,4));// right_bottom mini rectangle.
			cxt.cxt->FillRectangle(rc1, cxt.green);
		}
	}

	
	tbox_->WndProc(parent_,WM_PAINT,wp,lp);




	//DefPaintWndProc(parent_,WM_PAINT,wp,lp);


	mat.PopTransform();
}
void D2DCells::OnDXDeviceLost() 
{ 
	SCBAR(Vscbar_)->OnDXDeviceLost();
	//SCBAR(Hscbar_)->OnDXDeviceLost();
}
void D2DCells::OnDXDeviceRestored()  
{ 
	SCBAR(Vscbar_)->OnDXDeviceRestored();
	//SCBAR(Hscbar_)->OnDXDeviceRestored();
}

FRectFBoxModel D2DCells::VScrollbarRect( const FRectFBoxModel& rc )
{
	_ASSERT(rc.left == 0&&rc.top == 0);
	FRectFBoxModel xrc(rc);
	xrc.left = xrc.right - BARWIDTH;
	//xrc.top += (titlebar_enable_ ? TITLEBAR_HEIGHT : 0);
	xrc.bottom -= BARWIDTH;

	if (draw_mode_ & EXCELTITLE)
		xrc.top += COL_LABEL_H;

	if ( bar_->IsVisible() )
	{
		auto h = bar_->GetRect().Height();
		xrc.top += h;
	}

	xrc.bottom -= BARWIDTH;

	return xrc;
}
void D2DCells::SetCanvasSize( float cx, float cy )
{
	if (cy > 0 )
	{
		cy = max(rc_.Height(), cy );	
		SCBAR(Vscbar_)->SetTotalSize( cy );
	}
	if ( cx > 0 )
	{
		cx = max(rc_.Width(), cx );	
		//SCBAR(Hscbar_)->SetTotalSize( cx );
	}

	SCBAR(Vscbar_)->Visible();
	//SCBAR(Hscbar_)->Visible();

	auto& vinfo = SCBAR(Vscbar_)->Info();
		
	//auto& hinfo = SCBAR(Hscbar_)->Info();

	bool bl1 = true, bl2 = true;

	if ( vinfo.total_height <= rc_.Height())
	{
		SCBAR(Vscbar_)->Hide();
		bl1 = false;
	}
	/*if ( hinfo.total_height <= rc_.Width())
	{
		SCBAR(Hscbar_)->Hide();
		bl2 = false;
	}*/

	SCBAR(Vscbar_)->OtherHand(bl2);
	//SCBAR(Hscbar_)->OtherHand(bl1);

}
int D2DCells::InnerDefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam)
{
	int ret = Vscbar_->WndProc(d,message,wParam,lParam);
	//if ( ret == 0 )
	//	ret = Hscbar_->WndProc(d,message,wParam,lParam);
	return ret;
}


void D2DCells::UpdateScrollbar(D2DScrollbar* bar)
{
	auto& info = bar->Info();
	
	if ( info.bVertical )
		scrollbar_off_.height = info.position / info.thumb_step_c;
	//else
	//	scrollbar_off_.width = info.position / info.thumb_step_c;
}




///////////////////////////////////////////////////////////////////////////////////////////////
LPCWSTR ColumnABC(int* len, int val)
{
	static WCHAR cb[8];
	
	if ( val < 26 )
	{
		cb[0] = L'A' + val;
		cb[1] = 0;
		*len = 1;
	}
	else
	{
		int a = val / 26;

		cb[0] = L'A' + a-1;
		cb[1] = L'A' + val - a*26;
		cb[2] = 0;
		*len = 2;
	}
	return cb;
}
LPCWSTR RowNumber(int* len, int val)
{
	static WCHAR cb[8];
	StringCbPrintf(cb,8, L"%d", val+1);
	*len = wcslen(cb);
	return cb;
}


/////////////////////////////////////////////////////////////////////////////////////////
int D2DCellsControlbar::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() && !IsImportantMsg(message) ) 
		return 0;

	int ret = 0;
	
	
	
	switch( message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = *(d->cxt());

			D2DMatrix mat(cxt);	
			mat_ = mat.PushTransform();

			cxt.cxt->FillRectangle(rc_, cxt.ltgray);
			
			DefPaintWndProc(d,message,wp,lp);

			
			
			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_RESIZE:
		case WM_SIZE:
		{
			auto sz = GetParentControl()->GetRect().Size();

			rc_.SetSize(sz.width, -1);

			if ( message == WM_SIZE )
				DefPaintWndProc(d,message,wp,lp);
			return 0;
		}
		break;
		case WM_D2D_CELLS_SELECT_CHANGED:
		{
			auto p = dynamic_cast<D2DCells*>(parent_control_);

			tbox_->SetText(p->SelectCellName());

			d->redraw();
			ret = 1;
		}
		break;
	}


	if ( ret == 0 )
		ret = D2DControls::DefWndProc(d,message,wp,lp);


	return ret;

}
void D2DCellsControlbar::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	FRectFBoxModel txrc(7,10,100,30);

	txrc.Padding_.l = 5;

	auto& caret = Caret::GetCaret();
	D2CoreTextBridge*  ime_bridge =  dynamic_cast<D2DMainWindow*>(pacontrol->GetParentWindow())->GetImeBridge();
	tbox_ = new D2DTextbox(*ime_bridge, caret);
	tbox_->Create(this, txrc, VISIBLE, NONAME);
	tbox_->SetReadonly(true);
}