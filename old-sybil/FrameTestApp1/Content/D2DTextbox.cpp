﻿#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DTextbox.h"


using namespace V4;

#define KEY_SHIFT	0x1
#define KEY_CONTROL 0x2

static int RCfromPosition( V4::D2CoreTextBridge& br, int pos, int* prow, int* pcol );
static bool RCToPosition( D2CoreTextBridge& br, int target_row, int target_col, int* ppos );
static std::wstring _str_append( const std::wstring& str , int s, WCHAR ch );

static void BackGround(D2DContext& cxt, D2D1_RECT_F& rc )
{
	cxt.cxt->DrawRectangle(rc, cxt.black );
	cxt.cxt->FillRectangle(rc, cxt.white);
}


D2DTextbox::D2DTextbox(D2CoreTextBridge& bridge, Caret& ca):bridge_(bridge),back_(ColorF::White),fore_(ColorF::Black),caret_(ca)
{
	typ_ = TYP::SINGLELINE;
	shift_control_key_ = 0;
	IsReadOnly_ = false;
	bMouseSelectMode_ = false;
}
D2DTextbox::D2DTextbox(D2CoreTextBridge& bridge, TYP ty, Caret& ca):bridge_(bridge),back_(ColorF::White),fore_(ColorF::Black),caret_(ca)
{
	typ_ = ty;
	shift_control_key_ = 0;
	IsReadOnly_ = false;
	bMouseSelectMode_ = false;
}

static std::wstring RemoveStringByBACK(const std::wstring& str, int& spos, int& epos)
{
	_ASSERT( spos <= (int)str.length() );
					
	int acp = spos;
	int len = str.length();
				
	if ( 0 < acp )
	{
		int rmlen = max(1, epos - spos );

		spos = epos - rmlen;

		acp = spos;
														
		auto s1 = str.substr( 0, acp );

		if ( 0 < (len-(acp+rmlen)) )
		{
			auto s2 = str.substr( acp+rmlen, (len-(acp+rmlen)) );
			s1 += s2;
		}
		epos = spos;

		return s1;
	}
	return str;
}
static std::wstring RemoveStringByDELETE( const std::wstring& str, int& spos, int& epos )
{
	_ASSERT( spos <= (int)str.length() );
					
	int acp = spos;
	int len = str.length(); 
				
	if ( acp < len )
	{
		int rmlen = max(1, epos-spos );

		auto s1 = str.substr( 0, acp );
							
		if ( 0 < (len-(acp+rmlen)) )
		{
			auto s2 = str.substr( acp+rmlen, (len-(acp+rmlen)) );
			s1 += s2;
		}						
		
		epos = spos;
		return s1;
	}
	return str;
}

void D2DTextbox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow(pacontrol,rc,stat,name, controlid);

	ti_.fmt_ = parent_->cxt()->cxtt.textformat;
	ti_.wfac_ = parent_->cxt()->cxtt.wfactory;
	ti_.ontext_updated_ = std::bind( &D2DTextbox::OnTextUpdated, this );

	back_ground_ = BackGround;
	OnTextUpdated();
}

void D2DTextbox::OnTextUpdated()
{
	// textがupdateされたのでtextlayoutを作成する
	
	layout_.Release();
	auto hr = ti_.wfac_->CreateTextLayout( ti_.text.c_str(), ti_.text.length(), ti_.fmt_, ti_.rcTextbox.Width(), ti_.rcTextbox.Height(), &layout_ );

	DWRITE_LINE_METRICS lm; 
	UINT32 c;
	layout_->GetLineMetrics(&lm,1,&c);

	ti_.line_height = lm.height;
}

FRectF D2DTextbox::_rc() const
{
	return rc_.GetContentRect();
}
int D2DTextbox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			ti_.mat = mat_;

			FRectF rcb = rc_.GetBorderRect();
			mat.Offset(rcb.left, rcb.top); // border rect基準


			ComPTR<ID2D1SolidColorBrush> forebr;
			cxt.cxt->CreateSolidColorBrush( fore_, &forebr );

			FRectF rc = rcb.ZeroRect();
			this->back_ground_(cxt,rc);

			
			auto fmt = ti_.fmt_;

			FRectF rca = rc_.GetContentBorderBase(); 

			D2D1_DRAW_TEXT_OPTIONS opt = D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP;
			

			if ( ti_.decoration_typ == 0 )
			{								
				cxt.cxt->DrawTextLayout( rca.LeftTop(), layout_, forebr, opt );
			}
			else if ( ti_.decoration_start_pos != ti_.decoration_end_pos )
			{
				LPCWSTR str = ti_.text.c_str();
				int len = ti_.text.length();
				
				cxt.cxt->DrawText( str,  ti_.decoration_start_pos, fmt, rca, cxt.black,opt );
				

				FRectF rcb(rca);

				FRectF* rcc = ti_.rcChar().get();

				FPointF pt = rcc[ti_.decoration_start_pos].LeftTop();

				rcb.Offset( pt.x, pt.y );
				
				auto clr = cxt.black;
				
				if ( ti_.decoration_typ == 1 )
					clr = cxt.gray;
				else if ( ti_.decoration_typ == 10 )
					clr = cxt.bluegray;
				else if ( ti_.decoration_typ == 9 )
					clr = cxt.black;

				

				cxt.cxt->DrawText( str+ti_.decoration_start_pos,  ti_.decoration_end_pos-ti_.decoration_start_pos, fmt, rcb, clr, opt );

				// next line
				int r,c;
				RCfromPosition(bridge_, ti_.decoration_start_pos, &r, &c );
				
				int npos;
				if (RCToPosition(bridge_, r+1, 0, &npos ))
				{
					pt = rcc[npos].LeftTop();
					rca.Offset( pt.x, pt.y );
					cxt.cxt->DrawText( str+npos,  len-npos, fmt, rca, forebr, opt );
				}
			}

			DrawSelectArea( cxt );


			mat.PopTransform();
		}
		break; 
		case WM_LBUTTONDOWN:
		{
			if ( IsReadOnly_ ) return 0;
			
			FPointF ptd(lp);
			bMouseSelectMode_ = false;
			
			FPointF pt = mat_.DPtoLP(ptd);

			FRectF rc = _rc();
			if ( rc.PtInRect(pt ))
			{
				FPointF pt(pt.x - rc.left, pt.y - rc.top);

				FRectF* rc = ti_.rcChar().get(); 

				for( int i = 0; i < ti_.rcCharCnt; i++ )
				{
					if ( rc[i].PtInRect(pt)	)
					{
						ti_.sel_start_pos = ti_.sel_end_pos = i;
						bMouseSelectMode_ = true;
						break;
					}
				}

				if ( !bMouseSelectMode_ )
				{
					ti_.sel_start_pos = ti_.sel_end_pos = ti_.rcCharCnt;
					bMouseSelectMode_ = true;
				}

				bridge_.SetCaret(ti_.sel_start_pos, ti_.sel_end_pos);

				
				Activate(ti_.sel_start_pos);// Capture!!
				ret = 1;
			}
		} 
		break;
		case WM_MOUSEMOVE :
		{
			if ( IsReadOnly_ ) return 0;

			if (this == GetParentControl()->GetCapture() && bMouseSelectMode_ )
			{
				FPointF pt1 = mat_.DPtoLP(FPointF(lp));
				FRectF rcc = _rc();
				FPointF pt(pt1.x - rcc.left, pt1.y - rcc.top);

				FRectF* rc = ti_.rcChar().get();

				int xi = ti_.sel_start_pos;

				bool bl =  false;

				for (int i = 0; i < ti_.rcCharCnt; i++)
				{					
					if ( rc[i].PtInRect(pt) )
					{
						xi = i;				
						bl = true;
						break;
					}
				}

				if ( !bl )
				{
					if ( 0 < ti_.rcCharCnt && ti_.sel_start_pos < ti_.sel_end_pos ) //左から右へ 終端
						xi = ti_.rcCharCnt;
					else
						xi = ti_.sel_end_pos; 	//右から左へ	
				}
				
				
				ti_.sel_end_pos = xi;
				bridge_.SetCaret(ti_.sel_start_pos, ti_.sel_end_pos);

				ret = 1;
				

			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( IsReadOnly_ ) return 0;

			if (this == GetParentControl()->GetCapture() && bMouseSelectMode_)
			{
				bMouseSelectMode_ = false;
				ret = 1;
			}
		}
		break;
		case WM_KEYDOWN:
		{
			if ( IsReadOnly_ ) return 0;

			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			if ( bridge_.GetTarget() == this )
			{
				ret = 1;
				
				switch ( arg->VirtualKey )
				{
					case Windows::System::VirtualKey::Back:
					{							
						ti_.text = RemoveStringByBACK( ti_.text, ti_.sel_start_pos, ti_.sel_end_pos );
												
						
						bridge_.SetCaret(ti_.sel_start_pos);						
						bridge_.UpdateTextRect( _rc().Size() );
						OnTextUpdated();
					}
					break;
					case Windows::System::VirtualKey::Delete:
					{
						ti_.text =  RemoveStringByDELETE( ti_.text, ti_.sel_start_pos, ti_.sel_end_pos );
						
						bridge_.SetCaret(ti_.sel_start_pos);
						bridge_.UpdateTextRect( _rc().Size());
						OnTextUpdated();
					}
					break;

					case Windows::System::VirtualKey::Left:
					{
						int acp = ti_.sel_start_pos;
						

						if ( shift_control_key_ & KEY_SHIFT )
						{
							if ( caret_.is_start_change_ == 0 )
								caret_.is_start_change_ = 1;
							
							{
								if ( caret_.is_start_change_ == 1 )
									ti_.sel_start_pos = max( 0, acp-1);
								else
									ti_.sel_end_pos = max( ti_.sel_end_pos-1 , ti_.sel_start_pos) ;

								bridge_.SetCaret(ti_.sel_start_pos, ti_.sel_end_pos);

								if ( ti_.sel_start_pos == ti_.sel_end_pos )
									caret_.is_start_change_ = 0;
							}
						}
						else
						{
							ti_.sel_start_pos = max( 0, acp-1);
							caret_.is_start_change_ = 0;
							ti_.sel_end_pos = ti_.sel_start_pos;
							bridge_.SetCaret(ti_.sel_start_pos);
						}
					}
					break;
					case Windows::System::VirtualKey::Right:
					{
						int acp = ti_.sel_end_pos;

						
						if ( shift_control_key_ & KEY_SHIFT )
						{							
							if ( caret_.is_start_change_ == 0 )
								caret_.is_start_change_ = -1;

							if ( caret_.is_start_change_ == -1 )
									ti_.sel_end_pos = min( (int)ti_.text.length(), max( 0, acp+1));
								else
									ti_.sel_start_pos++ ;

							bridge_.SetCaret(ti_.sel_start_pos, ti_.sel_end_pos);

							if ( ti_.sel_start_pos == ti_.sel_end_pos )
									caret_.is_start_change_ = 0;
						}
						else
						{
							ti_.sel_end_pos = min( (int)ti_.text.length(), max( 0, acp+1));

							caret_.is_start_change_ = 0;
							ti_.sel_start_pos = ti_.sel_end_pos;
							bridge_.SetCaret(ti_.sel_start_pos);
						}

					}
					break;
					case Windows::System::VirtualKey::Home:
					{
						int r, c, pos;
						RCfromPosition(bridge_, ti_.sel_start_pos, &r, &c);

						if (RCToPosition(bridge_, r, 0, &pos))
						{
							ti_.sel_end_pos = ti_.sel_start_pos = pos;
							bridge_.SetCaret(ti_.sel_start_pos);
						}
						else
						{
							ti_.sel_end_pos = ti_.sel_start_pos = 0;
							bridge_.SetCaret(ti_.sel_start_pos);
						}
						
					}
					break;
					case Windows::System::VirtualKey::End:
					{
						int r,c,pos;
						RCfromPosition( bridge_, ti_.sel_start_pos, &r, &c );
						
						if ( RCToPosition(bridge_, r+1, 0, &pos ))
						{
							ti_.sel_end_pos = ti_.sel_start_pos = pos-1;
							bridge_.SetCaret(ti_.sel_start_pos);
						}
						else
						{
							ti_.sel_end_pos = ti_.sel_start_pos = ti_.text.length();
							bridge_.SetCaret(ti_.sel_start_pos);
						}
					}
					break;
					case Windows::System::VirtualKey::Shift:
						shift_control_key_ |= KEY_SHIFT;
					break;
					case Windows::System::VirtualKey::Control:
						shift_control_key_ |= KEY_CONTROL;
					break;
					case Windows::System::VirtualKey::Escape:
					{
						bool bl = true;

						WParameter wp;
						wp.sender = this;
						wp.prm = &bl;
						GetParentControl()->WndProc(parent_, WM_D2D_ESCAPE_FROM_CAPTURED, (INT_PTR)&wp, nullptr);
						if ( bl == true )
						{
							UnActivate();
						}

						
					}
					break;
					case Windows::System::VirtualKey::Enter:
					{
						if ( typ_ == TYP::MULTILINE )
						{
							//TRACE( L"Windows::System::VirtualKey::Enter pos=%d\n", ti_.sel_start_pos );

							ti_.text = _str_append( ti_.text, ti_.sel_start_pos, L'\n' );

							ti_.sel_start_pos++;
							ti_.sel_end_pos = ti_.sel_start_pos;

							bridge_.NotifyTextChanged( 1 );

							bridge_.UpdateTextRect( _rc().Size());
						}
						else
						{
							OnTextUpdated();

							WParameter wp;
							wp.sender = this;
							wp.prm = (LPVOID)ti_.text.c_str();
							
							parent_control_->WndProc( parent_, WM_D2D_TEXTBOX_CHANGED, (INT_PTR)&wp, nullptr );
						}
					}
					break;
					case Windows::System::VirtualKey::Up:
					{						
						if ( ti_.sel_start_pos > 0 )
						{
							int row = 0, col = 0;
							RCfromPosition( bridge_, ti_.sel_start_pos, &row, &col );

							if ( row > 0 )
							{
								int target_row = row - 1;
								int target_col = col;

								int newpos;

								bool bl = RCToPosition( bridge_, target_row, target_col, &newpos );
								
								if ( bl )
								{
									ti_.sel_end_pos = ti_.sel_start_pos = newpos;
									bridge_.SetCaret(ti_.sel_start_pos);
								}
							}
						}
					}
					break;
					case Windows::System::VirtualKey::Down:
					{						
						int row = 0, col = 0;
						RCfromPosition( bridge_, ti_.sel_start_pos, &row, &col );

						
						{
							int target_row = row + 1;
							int target_col = col;

							int newpos;

							bool bl = RCToPosition( bridge_, target_row, target_col, &newpos );
								
							if ( bl )
							{
								ti_.sel_end_pos = ti_.sel_start_pos = newpos;
								bridge_.SetCaret(ti_.sel_start_pos);
							}						
						}						
					}
					break;
				}
				
			}
		}
		break;
		case WM_KEYUP:
		{
			if ( IsReadOnly_ ) return 0;

			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			if ( bridge_.GetTarget() == this )
			{
				ret = 1;
				
				switch ( arg->VirtualKey )
				{
					case Windows::System::VirtualKey::Shift:
						shift_control_key_ &= ~KEY_SHIFT;
					break;
					case Windows::System::VirtualKey::Control:
						shift_control_key_ &= ~KEY_CONTROL;
					break;
				}
			}

		}
		break;
		case WM_CHAR:
		{
			if ( IsReadOnly_ ) return 0;
			ret = ( bridge_.GetTarget() == this ? 1 : 0);

		}
		break;
		case WM_D2D_TEXTBOX_SETTEXT:
		{
			WParameter* pr = (WParameter*)wp;

			if ( pr->target == this )
			{
				auto str = (LPCWSTR)pr->prm;

				SetText(str);
				d->redraw();
				ret = 1;
			}
		}
		break;
		case WM_D2D_TEXTBOX_GETTEXT:
		{
			WParameter* pr = (WParameter*)wp;
			if ( pr->target == this )
			{
				pr->prm = (LPVOID)ti_.text.c_str();

				ret = 1;
			}
		}
		break;

	}
	return ret;

}
void D2DTextbox::Activate(int init_pos)
{	
	auto p = GetParentControl()->GetCapture();
	if ( p != this )
	{		
		ti_.rcTextbox = mat_.LPtoDP(rc_.GetContentRect());  // device coordinate

		ti_.sel_start_pos = init_pos;
		ti_.sel_end_pos = init_pos;
		ti_.decoration_start_pos = ti_.decoration_end_pos = 0;
		ti_.decoration_typ = 0;
		ti_.mat = mat_;
		ti_.rcTextboxLog = rc_.GetContentRect();

		shift_control_key_ = 0;

		caret_.Activate( ti_ );

		bridge_.Activate( &ti_, this );

		//if ( GetParentControl()->GetCapture() )
		GetParentControl()->ReleaseCapture();

		GetParentControl()->SetCapture(this);
	}
	else
	{
		ti_.sel_start_pos = init_pos;
		ti_.sel_end_pos = init_pos;
		caret_.Activate( ti_ );
		bridge_.Activate( &ti_, this );
	}
}
void D2DTextbox::UnActivate()
{
	if ( GetParentControl()->GetCapture() == this )
		GetParentControl()->ReleaseCapture();


}
void D2DTextbox::SetReadonly( bool IsReadOnly )
{
	IsReadOnly_ = IsReadOnly;

	//SetText( ti_.text.c_str());
}
void D2DTextbox::SetText(LPCWSTR txt)
{
	if ( !IsReadOnly_ )
	{
		bool bl = ( GetParentControl()->GetCapture() != this );
		
		ti_.text = txt; 
		ti_.sel_end_pos = ti_.sel_start_pos = 0;

		Activate(ti_.sel_start_pos);
		
		bridge_.SetCaret(ti_.sel_start_pos);


		bridge_.UpdateTextRect(_rc().Size());

		UnActivate();

		OnTextUpdated();
	
		
	}
	else
	{
		ti_.text = txt;		
		ti_.rcTextbox = _rc().ZeroRect();						
		OnTextUpdated();
	}
	
}

void D2DTextbox::SetFont( const FontInfo& cf, int align )
{
	auto prv = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING;
	
	if ( ti_.fmt_ && align < 0 )
		prv = ti_.fmt_->GetTextAlignment();

	ti_.fmt_ = cf.CreateFormat( ti_.wfac_ );

	if ( align < 0 )
		ti_.fmt_->SetTextAlignment(prv);
	else
		SetAlign(align);
}
void D2DTextbox::SetAlign( int typ )
{
	s_SetAlign( ti_.fmt_, typ );
}
void D2DTextbox::s_SetAlign( IDWriteTextFormat* fmt, int typ )
{
	_ASSERT( fmt );
	switch( typ )
	{
		case 0:
			fmt->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING );
		break;
		case 1:
			fmt->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER );
		break;
		case 2:
			fmt->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING );
		break;
	};	
}


void D2DTextbox::OnReleaseCapture( int layer )
{
	D2DControl::OnReleaseCapture(layer);

	if (bridge_.GetTarget() == this)
	{
		caret_.UnActivate();

		ti_.init();

		bMouseSelectMode_ = false;

		bridge_.UnActivate();
	}
}


void D2DTextbox::DrawSelectArea(D2DContext& cxt)
{	
	if ( ti_.sel_start_pos == ti_.sel_end_pos ) return;

	D2DMatrix mat(cxt);
	mat.PushTransform();
	
	FRectF* prc = ti_.rcChar().get();

	int si = min(ti_.sel_start_pos, ti_.sel_end_pos); //decoration_end_pos);
	int ei = max(ti_.sel_start_pos, ti_.sel_end_pos);


	mat.Offset( rc_.Padding_.l, rc_.Padding_.t );

	for( int i = si; i < ei; i++ )
	{
		cxt.cxt->FillRectangle( prc[i], cxt.halftone );
	}
	
	mat.PopTransform();
}

static int RCfromPosition( V4::D2CoreTextBridge& br, int pos, int* prow, int* pcol )
{
	FRectF* rc = br.info_->rcChar().get();

	int rccnt = br.info_->rcCharCnt;
	
	float prtop = 0;
	int row = 0, col = 0;
	for( int i = 0; i < rccnt; i++ )
	{
		if ( i == pos )
			break;

		col++;
		if ( rc[i].top != prtop )
		{
			row++;
			prtop = rc[i].top;
			col = 0;									
		}																	
	}
	
	*prow = row;
	*pcol = col;

	int row_last_col_pos = 0;
	for (int i = col; i < rccnt; i++)
	{
		if (rc[i].top != prtop)
		{
			row_last_col_pos = i;
			break;
		}
	}

	return row_last_col_pos;
}

static bool RCToPosition( D2CoreTextBridge& br, int target_row, int target_col, int* ppos )
{
	bool ret = false;
	FRectF* rc = br.info_->rcChar().get();
	const int len = br.info_->text.length();
	
	int j = 0;
	int r = 0, c = 0;
	float prtop = 0;

	float cw = 0;
	int col = 0;
	for( j = 0; j < len; j++ )
	{
		cw = rc[j].Width();
		if ( rc[j].top != prtop && cw )
		{													
			prtop = rc[j].top;
		}
		else if (rc[j].top != prtop && cw == 0)
		{
			r++;
		}

		if (target_row == r )
		{
			ret = true;
			prtop = rc[j].top;

			if (cw == 0 )
				j++;
			break;
		}
	}

	for( ; j < len; j++ )
	{
		if ( rc[j].top != prtop )
		{										
			break;
		}

		if ( target_col == col )
			break;

		if ( rc[j].Width())
			col++;
	}
	
	*ppos = j;
	
	
	return ret;
}
static std::wstring _str_append( const std::wstring& str , int s, WCHAR ch )
{
	if ( s < (int)str.length() )
	{

		auto s1 = str.substr( 0, s );

		auto s2 = str.substr( s, str.length()-s );

		s1 += ch;

		return s1+s2;
	}
	else if ( s == (int)str.length())
	{
		return str + ch;
	}
	else
		return L"str_append_error";
}

/////////////////////////////////////////////////////////////////////////////////////////////////

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
		
