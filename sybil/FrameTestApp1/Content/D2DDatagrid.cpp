#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDatagrid.h"
#include "sybil.h"

#define TITLE_HEIGHT 26.0f
#define ROW_HEIGHT 22.0f
#define X_TRIM 3
#define SCBAR(x) ((D2DScrollbar*)x.get())



namespace V4 {


D2DDatagridLight::~D2DDatagridLight()
{
	Clear();
}
void D2DDatagridLight::Clear()
{
	for( auto& it : cols_ )
		::SysFreeString(it.title);
	for( auto& it : rows_ )
	{
		for( int i = 0; i < it->colcnt; i++ )
			::SysFreeString( it->values[i] );

		delete [] it->values;
	}

	cols_.clear();
	rows_.clear();

	ClearRowsLayout();

}
int D2DDatagridLight::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			mat.Offset( rcborder.left, rcborder.top );		

			

			mat.PushTransform();
			{
				//DrawTitle( cxt );
				//mat.Offset( 0, TITLE_HEIGHT );

				mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height+TITLE_HEIGHT );
						
				mat.CopyTransform(&matrow_);
			
				DrawList( cxt );
			}
			mat.PopTransform();

			mat.PushTransform();
			DrawTitle( cxt );
			mat.PopTransform();

			Vscbar_->WndProc(d,WM_PAINT,0,nullptr);
			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONDOWN :
		{	
			FPointF pt = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt))
			{		
				if ( GetRowsArea().PtInRect(pt) )
				{
					selected_row_ = -1;
					FPointF ptrow = matrow_.DPtoLP( lp);
					selected_row_ = (int)(ptrow.y / ROW_HEIGHT);
				
					ret = 1;
					parent_control_->SetCaptureByChild(this);
				}
				else
				{
					ret = Vscbar_->WndProc(d,message,wp,lp);
				}
			}
		}
		break;

		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt) || IsCaptured() )
			{	
				
				if ( rc_.top<pt.y && pt.y < rc_.top + TITLE_HEIGHT )
				{
					D2DMainWindow::SetCursor(CURSOR_SizeWestEast);
				}
				else
				{
					D2DMainWindow::SetCursor(CURSOR_ARROW);					
				}

				ret = Vscbar_->WndProc(d,message,wp,lp);
				ret = 1;
			}

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( parent_control_->GetCapture() == this )
			{
				parent_control_->ReleaseCapture();

				ret = 1;
			}

			if ( ret == 0 )
				ret = Vscbar_->WndProc(d,message,wp,lp);
		}
		break;
		case WM_SIZE:
		case WM_D2D_INIT_UPDATE:
		{
			rc_ = parent_control_->GetInnerRect();

			FRectF rcc = rc_;

			FRectFBoxModel xrc = D2DChildFrame::VScrollbarRect(rcc);

			xrc.bottom -= TITLE_HEIGHT;

			Vscbar_->SetRect(xrc);
			((D2DScrollbar*)Vscbar_.get())->SetRowHeight( ROW_HEIGHT );

			D2DControls::DefWndProc(d,message,wp,lp);
		}
		break;

		default :
			ret = D2DControls::DefWndProc(d,message,wp,lp);
	}


	return ret;


}

void D2DDatagridLight::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
	selected_row_ = -1;
	col_sum_width_ = 0;

	FRectFBoxModel xrc = D2DChildFrame::VScrollbarRect(rc.GetContentRectZero());
	D2DScrollbar* Vscbar = new D2DScrollbar();
	Vscbar->Create(parent,this,xrc,VISIBLE,NONAME );
	Vscbar_ = controls_[0];
	controls_.clear();
	SCBAR(Vscbar_)->Hide();
}
D2DDatagridLight::Row* D2DDatagridLight::NewRow()
{
	Row* r = new Row();
	r->colcnt = cols_.size();
	r->values = new BSTR[ r->colcnt ];
	return r;
}
void D2DDatagridLight::AddRow(Row* row )
{
	rows_.push_back( row );
}

int D2DDatagridLight::RowsCount()
{
	return (int)rows_.size();
}
void D2DDatagridLight::AddCol( ColInfo& col )
{
	cols_.push_back(col);

	col_sum_width_ += col.width;
}

void D2DDatagridLight::DrawTitle(D2DContext& cxt )
{
	FRectF rca(0,0,rc_.Width(),TITLE_HEIGHT );

	cxt.cxt->FillRectangle( rca, cxt.white );

	FRectF rc( X_TRIM,0,0,TITLE_HEIGHT );

	for( auto& it : cols_ )
	{
		BSTR bs = it.title;
		float w = it.width;

		rc.right = rc.left + w;
		
		cxt.cxt->DrawText( bs, ::SysStringLen(bs), cxt.cxtt.textformat, rc, cxt.black  );

		rc.Offset( w, 0 );
	}
	
	/*D2DMatrix mat(cxt);	
	mat.GetTransform();
	mat.Offset( 0, TITLE_HEIGHT );*/

}
// 



void D2DDatagridLight::DrawList(D2DContext& cxt )
{
	if ( rows_layout_.empty())
		GenRowsLayout();

	D2DMatrix mat(cxt);	
	mat.GetTransform();

	int ir = 0;
	for( auto& rl : rows_layout_ )
	{		
		FRectF rc( X_TRIM,0,0,TITLE_HEIGHT );
		int ic = 0;

		auto clr = cxt.black;

		if ( ir == selected_row_ )
		{
			clr = cxt.white;
			FRectF rca(rc);
			rca.left = 0;
			rca.right = rca.left + rc_.Width();

			cxt.cxt->FillRectangle( rca, cxt.bluegray );
		}
		
		
		for( auto& it : cols_ )
		{
			//BSTR bs = r->values[ic];
			float w = it.width;
			
			rc.right = rc.left + w;

			//cxt.cxt->DrawText( bs, ::SysStringLen(bs), cxt.cxtt.textformat, rc, clr  );

			cxt.cxt->DrawTextLayout( rc.LeftTop(), rl->values[ic], clr );


			rc.Offset( w, 0 );
			ic++;
		}

		mat.Offset( 0, ROW_HEIGHT );
		ir++;
	}
}
//DLLEXPORT bool CreateShortTextLayout( IDWriteFactory* fac, IDWriteTextFormat* wl, LPCWSTR str, int strlen, float width, IDWriteTextLayout** ret );

D2DDatagridLight::RowLayout* D2DDatagridLight::GenRowLayout( Row* r, std::vector<ColInfo>& coli )
{	
	RowLayout* rl = new RowLayout();
	rl->colcnt = r->colcnt;
	rl->values = (IDWriteTextLayout**)new INT_PTR[rl->colcnt];
	
	IDWriteFactory* fac = this->GetDWFactory();
	IDWriteTextFormat* fmt = this->GetTextFormat();

	for( int ic = 0; ic < r->colcnt; ic++ )
	{
		ComPTR<IDWriteTextLayout> tl;
		sybil::CreateShortTextLayout( fac, fmt, r->values[ic], ::SysStringLen(r->values[ic]), coli[ic].width, &tl );

		tl->AddRef();
		rl->values[ic] = tl;
	}

	return rl;
}

void D2DDatagridLight::ClearRowsLayout()
{
	for( auto& it : rows_layout_ )
	{	
		for( int i = 0; i < it->colcnt; i++ )
			it->values[i]->Release();

		delete [] it->values;

		delete it;
	}
	rows_layout_.clear();
}

void D2DDatagridLight::GenRowsLayout(int col)
{
	if ( col < 0 )
	{
		ClearRowsLayout();

		for( auto& it : rows_ )
		{		
			auto lay = GenRowLayout(it, cols_ );
			rows_layout_.push_back(lay);
		}

		SCBAR(Vscbar_)->SetTotalSize( rows_layout_.size() * ROW_HEIGHT );
	}
	else if ( col < (int)cols_.size() )
	{
		IDWriteFactory* fac = this->GetDWFactory();
		IDWriteTextFormat* fmt = this->GetTextFormat();
		int ir = 0;
		for( auto& it : rows_layout_ )
		{
			auto old = it->values[col];
			old->Release();

			BSTR bs = rows_[ir]->values[col];

			ComPTR<IDWriteTextLayout> tl;
			sybil::CreateShortTextLayout( fac, fmt, bs, ::SysStringLen(bs), cols_[col].width, &tl );
			
			tl->AddRef();
			it->values[col] = tl;

			ir++;
		}
	}
}

void D2DDatagridLight::Update()
{
	ClearRowsLayout();

	Vscbar_->Visible();

	parent_->redraw();
}
void D2DDatagridLight::UpdateScrollbar(D2DScrollbar* bar)
{
	auto& info = bar->Info();
	
	if ( info.bVertical )
	{
		//scrollbar_off_.height = (info.position / info.thumb_step_c);
		scrollbar_off_.height = info.rowno * info.row_height;


		
	}
	else
	{
		scrollbar_off_.width = info.position / info.thumb_step_c;
	}

}
FRectF D2DDatagridLight::GetRowsArea()
{
	// 行の表示エリア

	FRectF rc = rc_.GetBorderRect();

	rc.top += TITLE_HEIGHT;

	if (!Vscbar_->IsHide())
		rc.right -= Vscbar_->GetRect().Size().width;
	
	return rc;
}


void D2DDatagridLight::OnDXDeviceLost() 
{ 
	D2DControls::OnDXDeviceLost();

	ClearRowsLayout();
}
void D2DDatagridLight::OnDXDeviceRestored()  
{ 
	D2DControls::OnDXDeviceRestored();

	GenRowsLayout();
}
};