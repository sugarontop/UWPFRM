#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DInputControl.h"


using namespace V4;
using namespace V4_XAPP1;



void D2DInputTextbox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, std::vector<InputRow>& rows, Init& init)
{
	D2DWindow* win = pacontrol->GetParentWindow();

	InnerCreateWindow(win,pacontrol,rc,stat, L"noname", -1);

	D2DContext* cxt = win->cxt();

	int i = 0;
	rows_.resize(rows.size());

	ComPTR<IDWriteTextFormat> tf, vf;

	cxt->wfactory->CreateTextFormat( init.fontnm, nullptr, 
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		init.title_font_height,
		DEFAULTLOCALE,
		&tf );

	cxt->wfactory->CreateTextFormat( init.fontnm, nullptr, 
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		init.title_font_height,
		DEFAULTLOCALE,
		&vf );

	 
	for(auto& it : rows)
	{				
		ComPTR<IDWriteTextLayout> tl, val;
		cxt->wfactory->CreateTextLayout( it.title, wcslen(it.title), tf, 100,1000, &tl ); 

		cxt->wfactory->CreateTextLayout( L"***", 3, vf, 100,1000, &val ); 

		Row r;
		r.row = i;
		r.typ = it.typ;
		r.title = tl;
		r.value = val;
		r.height = it.height;

		rows_[i++] = r;
	}

	tx_ = init.textbox;

	cell_width_.resize(init.width_cnt);
	for(i = 0; i < init.width_cnt; i++ )
	{
		cell_width_[i] = init.width[i];
	}



	FRectF txbox_rc;
	txbox_rc.SetPoint( cell_width_[0], 0 );
	txbox_rc.SetSize( cell_width_[1], rows[0].height );
	txbox_rc.Offset( rc_.left, rc_.top );
	tx_->SetRect( txbox_rc );

	static WParameter wp;
	wp.sender = this;
	wp.target = tx_;
	wp.prm = (LPVOID)L"this is test.";

	
	
	//tx_->WndProc( win, WM_D2D_TEXTBOX_SETTEXT, (INT_PTR)&wp, nullptr );
	win->PostMessage(WM_D2D_TEXTBOX_SETTEXT, (INT_PTR)&wp, nullptr );

}
int D2DInputTextbox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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


			OnPaint(cxt);


			mat.PopTransform();
		}
		break; 
		case WM_D2D_TEXTBOX_CHANGED :
		{
			WParameter* wwp = (WParameter*)wp;
			if ( wwp->sender == tx_ )
			{
				std::wstring s = (LPCWSTR)wwp->prm;


				auto rc = tx_->GetRect();

				rc.Offset(0,40);

				tx_->SetRect( rc );

				d->redraw();

				ret = 1;
			}
		}
		break;
		
	}
	   


	return ret;
}
void D2DInputTextbox::OnPaint(D2DContext& cxt)
{
	FPointF pt(0,0);
	for(auto& it : rows_)
	{	
		pt.x = 0;
		cxt.cxt->DrawTextLayout( pt, it.title, cxt.black );

		pt.x += cell_width_[0];

		pt.x += cell_width_[1];

		cxt.cxt->DrawTextLayout( pt, it.value, cxt.black );

		pt.y += it.height;
	}

}