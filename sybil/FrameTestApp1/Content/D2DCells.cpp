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


LPCWSTR ColumnABC(int* len, int val);
LPCWSTR RowNumber(int* len, int val);

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

			Draw( cxt, wp,lp );

			

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
				SetSelectCell(RCPos(pt3));
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

				FRectF rc = SelectCellF();
				rc.right = rc.left + 500;

				FRectFBoxModel rc1 = tbox_->GetRect();
				rc1.SetRect(rc.LeftTop(), rc.GetSize());

				tbox_->SetRect(rc1);
				tbox_->Visible();


				auto it = values_.find(sel_);
				if ( it != values_.end())
					tbox_->SetText( it->second.str.c_str() );
				else
					tbox_->SetText(L"");

				


				ret = 1;
			}
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			FSizeF sz = GetParentControl()->GetRect().Size();

			rc_.SetSize(sz);

			DefPaintWndProc(d,message,wp,lp);
			return 0;
		}
		break;
		case WM_D2D_RESIZE:
		{
			FSizeF sz = GetParentControl()->GetRect().Size();

			rc_.SetSize(sz);
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
					auto value = tbox_->GetText();

					Cell2 c;
					c.str = value;
					c.sz = CreateTextLayout(cxt, value.c_str(), value.length(), &c.txt);
					c.pt = CellPos(cellnm);						
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
				values_[sel_] = c;
				

				auto rc = sel_;
				rc.y = min(row_limit(), sel_.y+1);

				SetSelectCell(rc);

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

			ret = 1;
		}
		break;
		case WM_MOUSEWHEEL:
		{
			FPointF pt3 = mat_.DPtoLP(lp);

			if ( rc_.PtInRect(pt3) )// this == parent_control_->GetCapture())
			{
				Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;

				//arg->CurrentPoint->Properties->IsRightButtonPressed

				int delta = arg->CurrentPoint->Properties->MouseWheelDelta;
				ret = 1;
				d->redraw();
				//start_idx_ = max(0,min((int)items_.size()-1, (delta < 0 ? (int)start_idx_+1 : (int)start_idx_-1 )));

				//OnSelectChanged();

				ret = 1;
			}
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
	return CellName(sel_);
}

int D2DCells::col_limit()
{
	return 26+26;
}
int D2DCells::row_limit()
{
	return 1000;
}
D2DCells::RowCol D2DCells::RCPos( FPointF pt )
{
	RowCol rc;
	rc.x = max(0, (int)(pt.x / unit_w_));
	rc.y = max(0, (int)(pt.y / unit_h_));
	return rc;
}

LPCWSTR D2DCells::CellName(RowCol rc)
{
	static WCHAR nm[8];
	
	int len;
	auto cb = ColumnABC(&len,rc.x);

	StringCbPrintf(nm,8,L"%s%d",cb, rc.y+1); 
	return nm;
}




void D2DCells::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	unit_w_ = INIT_CELL_WIDTH;
	unit_h_ = INIT_CELL_HEIGHT;
	draw_mode_ = (DT)(DT::EXCELLINE | DT::EXCELTITLE | DT::NORMAL);

	PreDraw();

	FRectFBoxModel txrc(0,0,unit_w_,unit_h_);
	txrc.Padding_.l = TEXTBOX_TRIMMING_LEFT;
	txrc.Padding_.t = TEXTBOX_TRIMMING_TOP;

	auto& caret = Caret::GetCaret();
	D2CoreTextBridge*  ime_bridge =  dynamic_cast<D2DMainWindow*>(pacontrol->GetParentWindow())->GetImeBridge();
	tbox_ = new D2DTextbox(*ime_bridge, caret);
	tbox_->Create(this, txrc, 0, NONAME);

	bar_ = new D2DCellsControlbar();
	bar_->Create(this,FRectF(0,0,rc_.Width(), CONTROLBAR_HEIGHT), 0, NONAME );



	WParameter wp;
	wp.sender = this;
	wp.target = pacontrol;
	wp.no = 0;
	pacontrol->WndProc(parent_, WM_D2D_VSCROLLBAR_SHOW, (INT_PTR)&wp,nullptr);
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
		values_[rc] = c;
	}

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

	WndProc(parent_,WM_D2D_CELLS_SELECT_CHANGED,0,nullptr);
}

void D2DCells::Draw(D2DContext& cxt, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp )
{
	int cnt = 1+ (int)(rc_.Width() / unit_w_);
	int rcnt = 1+ (int)(rc_.Height() / unit_h_);
	
	D2DMatrix mat(cxt);	

	mat.PushTransform();


	if ( bar_->IsVisible())
	{
		bar_->WndProc(parent_,WM_PAINT,wp,lp);
		
		mat.Offset(0,CONTROLBAR_HEIGHT); // 下段に下げる
	}


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
		for(int r=0; r< min(row_limit(),rcnt); r++ )
		{			
			cxt.cxt->DrawRectangle( rc1, cxt.gray );

			auto br = (sel_.y == r ? cxt.gray : cxt.ltgray);
			cxt.cxt->FillRectangle( rc1, br );

			auto t = lefttitle_[r];
			cxt.cxt->DrawTextLayout(t.pt, t.txt, cxt.black);
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
			cxt.cxt->FillRectangle( FRectF(it.pt, it.sz), cxt.white);
			auto pt = it.pt;
			pt.x += TEXTBOX_TRIMMING_LEFT;
			pt.y += TEXTBOX_TRIMMING_TOP;
			cxt.cxt->DrawTextLayout(pt, it.txt, cxt.black);
		}

		// draw selected cell
		if (tbox_->IsHide())
		{
			FPointF pt = CellF(sel_); 

			//TRACE( L"%02f,%02f   rc=%d,%d\n", pt.x, pt.y, sel_.x, sel_.y);
			
			FRectF selrc(pt.x, pt.y, FSizeF(unit_w_,unit_h_));
		
			cxt.cxt->DrawRectangle(selrc, cxt.blue);
		
			FRectF rc1(selrc.right-2,selrc.bottom-2,FSizeF(4,4));// right_bottom mini rectangle.
			cxt.cxt->FillRectangle(rc1, cxt.blue);
		}
	}

	
	tbox_->WndProc(parent_,WM_PAINT,wp,lp);




	//DefPaintWndProc(parent_,WM_PAINT,wp,lp);


	mat.PopTransform();
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
		case WM_SIZE:
		{
			auto sz = GetParentControl()->GetRect().Size();

			rc_.SetSize(sz.width, -1);

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