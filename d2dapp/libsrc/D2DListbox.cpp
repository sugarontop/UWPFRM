#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
//#include "higgsjson.h"
using namespace V4;

#define BTN_WIDTH 13

D2DDropDownListbox::D2DDropDownListbox()
{

}
void D2DDropDownListbox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);
	selected_idx_ = -1;
}
void D2DDropDownListbox::AddItem( LPCWSTR key, LPCWSTR value )
{
	items_[key] = value;
	
	Item x;
	x.idx = (int)ar_.size();
	x.key = key;

	ComPTR<IDWriteTextLayout> layout;

	auto wf = parent_->cxt()->cxtt.wfactory;
	auto tf = parent_->cxt()->cxtt.textformat;
	wf->CreateTextLayout( value, wcslen(value), tf, rc_.Width(), rc_.Height(), &layout );


	x.layout = layout;
	ar_.push_back(x);	
}

std::wstring D2DDropDownListbox::Value(int idx)  
{ 	
	std::wstring key = ar_[idx].key;
	
	return items_[key];
}
void D2DDropDownListbox::Clear()
{
	for( auto& it : ar_ )
		it.layout->Release();

	ar_.clear();
	items_.clear();
}
void D2DDropDownListbox::SetSelectIndex( int idx )
{
	int new_idx = (idx < (int)ar_.size() ? idx : selected_idx_ );

	if ( new_idx != selected_idx_ )
	{
		selected_idx_ = new_idx;

		WParameter wp;
		wp.no = selected_idx_;
		wp.sender = this;

		GetParentWindow()->SendMessage(WM_D2D_LB_EVNT_SELECT_CHANGE, (INT_PTR)&wp, nullptr );
	}
}


int D2DDropDownListbox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && !IsImportantMsg(message) ) 
	{
		if ( message != WM_D2D_LB_SET_SELECT_IDX && message != WM_D2D_LB_ADDITEM && 
			 message != WM_D2D_LB_ADD_ITEMS_JSON && message != WM_D2D_LB_CLEAR)
		{
			if (IsCaptured())
				GetParentControl()->ReleaseCapture();
			return 0;
		}
	}

	int ret = 0;
	
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			cxt.cxt->DrawRectangle( rc_, cxt.black );
			cxt.cxt->FillRectangle( rc_, cxt.white );
			if ( selected_idx_ > -1 )
			{				
				cxt.cxt->DrawTextLayout( rc_.GetContentRect().LeftTop(), ar_[selected_idx_].layout, cxt.black );
			}
			
			FRectF rc(rc_);
			rc.left = rc.right - BTN_WIDTH;
			cxt.cxt->FillRectangle( rc, cxt.ltgray ); // button

			DefPaintWndProc(d,message,wp,lp);

			mat.PopTransform();
			return 0;
		}
		break;

		case WM_LBUTTONDOWN:
		{	
			FPointF pt3 = mat_.DPtoLP(lp);

			if ( rc_.PtInRect(pt3))
			{
				FRectF rc(rc_);
				rc.Offset( 0, rc.Height());
				rc.top += 1.0f;
				rc.bottom = rc.top;

				InnerListbox* ls = new InnerListbox();
				ls->Create( this, rc, VISIBLE, NONAME,-1, selected_idx_);


				rc.bottom = rc.top;
				for( auto& it : ar_ )
				{
					ls->ar_.push_back(it.layout);

					DWRITE_TEXT_METRICS dm;
					it.layout->GetMetrics(&dm);

					rc.bottom += dm.height;
				}

				ls->SetRect(rc);

				


				SetCapture(ls);
				ret = 1;
			}
		}
		break;

		

		/*case WM_D2D_LB_ADD_ITEMS_JSON:
		{
			LPCWSTR json = (LPCWSTR)wp;

			std::vector<HiggsJson::Higgs> ar;
			if ( HiggsJson::ParseList(json,ar) )
			{
				for( auto& it : ar )
				{
					auto s = HiggsJson::ToStr(it);
					AddItem(s.c_str(),s.c_str());
				}
			}
			ret = 1;
		}
		break;*/
		case WM_D2D_LB_ADDITEM:
		{
			WParameterString* ws = (WParameterString*)wp;			
			AddItem(ws->str1, ws->str2);

			::SysFreeString(ws->str1); ws->str1 = nullptr;
			::SysFreeString(ws->str2); ws->str2 = nullptr;

			ret = 1;
		}
		break;
		case WM_D2D_LB_GET_ITEM:
		{
			WParameterString* ws = (WParameterString*)wp;

			if ( -1 < ws->idx && ws->idx < (int)ar_.size())
			{
				auto key = BSTRPtr(ar_[ws->idx].key.c_str()).detach();
				auto value = BSTRPtr(items_[key].c_str()).detach();
			
				ws->str1 =  key;
				ws->str2 = value;
			}
			ret = 1;
		}
		break;
		case WM_D2D_LB_COUNT:
		{
			int* r = (int*)wp;
			*r = (int)ar_.size();
			ret = 1;
		}
		break;
		case WM_D2D_LB_GET_SELECT_IDX:
		{
			int* r = (int*)wp;
			*r = (int)selected_idx_;
			ret = 1;
		}
		break;
		case WM_D2D_LB_SET_SELECT_IDX:
		{
			int idx = (int)wp;
			SetSelectIndex(idx);
			ret = 1;
		}
		break;
		case WM_D2D_LB_CLEAR:
		{
			Clear();			
			ret = 1;
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch ( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Escape:
				{
					bool bl = true;

					WParameter wp;
					wp.sender = this;
					wp.prm = &bl;
					GetParentControl()->WndProc(parent_, WM_D2D_ESCAPE_FROM_CAPTURED, (INT_PTR)&wp, nullptr);

					if ( bl == true )
					{
						//UnActivate();
					}

					ret = 1;
				}
				break;
			}
		}
		break;
	}

	if ( ret == 0 )
		ret = DefWndProc(d,message,wp,lp);

	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////

InnerListbox::InnerListbox()
{

}
void InnerListbox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name, int local_id, int selidx)
{
	
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);


	floadbr_ = CreateBrush( *parent_->cxt(), D2RGBA(233,0,0,100));
	float_idx_ = selidx;
	md_ = 0;

}
int InnerListbox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && !IsImportantMsg(message) ) 
		return 0;

	int ret = 0;
	
	
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			cxt.cxt->FillRectangle(rc_, cxt.white );

			FPointF pt = rc_.LeftTop();
			
			int k = 0;
			for( auto& it :ar_ )
			{
				cxt.cxt->DrawTextLayout(pt, it, cxt.black );
	

				DWRITE_TEXT_METRICS dm;
				it->GetMetrics(&dm);

				if ( k == float_idx_ )
				{
					FRectF rc(pt, FSizeF(rc_.Width(), dm.height) );
					cxt.cxt->FillRectangle(rc, floadbr_ );	
				}
				pt.y += dm.height;				
				k++;
			}

			mat.PopTransform();
			return 0;			
		}
		break;

		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			md_ = 1;
			if (!rc_.PtInRect(pt3))
			{
				Close();
			}
			ret = 1;
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt3 = mat_.DPtoLP(lp);

			if ( md_ == 1 )
			{
				Close(false);
				
				if ( rc_.PtInRect(pt3))
				{
					auto ls = dynamic_cast<D2DDropDownListbox*>(GetParentControl());
					if ( ls )
						ls->SetSelectIndex(float_idx_);
				}

				Close(true);
				ret = 1;
			}			
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt3))
			{
				FPointF pt = rc_.LeftTop();
				FRectF rc(pt.x, pt.y, FSizeF(rc_.Width(), 0));			
				int i = 0;
				for( auto& it :ar_ )
				{
					DWRITE_TEXT_METRICS dm;
					it->GetMetrics(&dm);
					
					rc.bottom = rc.top + dm.height;

					if ( rc.PtInRect(pt3))
					{						
						if ( float_idx_ != i )
						{
							float_idx_ = i;
							ret = 1;
							d->redraw();
							break;
						}
					}							

					rc.Offset(0,dm.height);
					i++;
				}
				ret = 1;

				d->redraw();
			}			
		}
		break;


		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs ^args = (Windows::UI::Core::KeyEventArgs ^)lp;

			if ( args->VirtualKey == Windows::System::VirtualKey::Escape )
			{
				Close();

				ret = 1;				
			}
		}
		break;

	}
	return ret;
}


void InnerListbox::Close(bool bDestroy)
{
	if ( GetParentControl()->GetCapture() == this )
	{
		GetParentControl()->ReleaseCapture();
	}
	if ( bDestroy )
		DestroyControl();

}
////////////////////////////////////////////////////////////////////////////////////////

ListboxItemString::ListboxItemString(D2DContext& cxt, LPCWSTR txt)
{	
	CreateTextLayout(cxt, txt, wcslen(txt), &layout_);
}
FRectF ListboxItemString::GetRect()
{
	DWRITE_TEXT_METRICS m;
	layout_->GetMetrics(&m);

	return FRectF(0,0,m.width+5,m.height+5);
}
void ListboxItemString::Draw(D2DContext& cxt)
{
	FPointF pt(3,3);
	cxt.cxt->DrawTextLayout( pt, layout_, cxt.black);
}
void ListboxItemString::Clear()
{
	if ( layout_ )
		layout_.Release();
	
}
void ListboxItemString::SetText(ComPTR<IDWriteTextLayout> layout)
{
	Clear();
	layout_ = layout;
}
//////////////////////////////
#define BARWIDTH 16

void D2DListbox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name,TYP typ, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);
	typ_ = typ;
	float_idx_ = -1;
	
	items_height_ = 0;
	md_ = 0;
	start_view_row_ =0;

	clr_float_ = D2RGBA(192, 192, 192, 180);
	clr_select_ = D2RGBA(192, 192, 192, 100);
}
int D2DListbox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && !IsImportantMsg(message) ) 
		return 0;

	int ret = 0;
	
	switch( message )
	{
		case WM_PAINT:
		{
			CXTM(d)

			mat_ = mat.PushTransform();

			
			auto rcc = rc_.GetContentRect();
			
			if (stat_ & BORDER)
			{
				auto rcb = rc_.GetBorderRect();
				cxt.cxt->FillRectangle(rcb, cxt.black);
			}

			D2DRectFilter df(cxt, rc_);

			cxt.cxt->FillRectangle(rcc, cxt.white);

			FPointF pt = rc_.LeftTop();

			mat.Offset( pt.x, pt.y );


			mat.PushTransform();
			{
				int k = 0;

				mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height);

				float h = 0;

				for( auto it = ar_.begin()+start_view_row_; it < ar_.end(); it++ )
				{
					(*it)->Draw(cxt);
				
					auto hi = (*it)->GetRect().Height();
				

					if ( (*it)->IsSelect() )
					{
						FRectF rc(0,0,rc_.Width(), hi );
						auto br = CreateBrush(cxt, clr_select_);
						cxt.cxt->FillRectangle(rc, br );	
					}
					else if ( k == float_idx_ )
					{
						FRectF rc(0,0,rc_.Width(), hi );

						auto br = CreateBrush(cxt, clr_float_);
						cxt.cxt->FillRectangle(rc, br );	
					}
									   
					mat.Offset(0,hi);

					h += hi;

					k++;
				}
			}
			mat.PopTransform();

			if ( vbar_ )
			{
				mat.Offset( rc_.Width()-BARWIDTH, 0 );
				vbar_->WndProc(d,message,wp,lp);
			}
			mat.PopTransform();
			return 0;			

		}
		break;
		case WM_D2D_NCHITTEST:
		{			
			LOGPT(pt3,wp);

			if (rc_.PtInRect(pt3))
			{
				ret = HTCLIENT;
			}
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
			else
			{
				this->float_idx_ = -1;

			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			md_ = 1;
			
			if ( vbar_ )
			{
				auto rc = vbar_->GetRect();
				rc.Offset(rc_.left+rc_.Width()-BARWIDTH, rc_.top);

				if (rc.PtInRect(pt3))
				{
					vbar_->WndProc(d,message,wp,lp); // “à•”‚Åcapture‚³‚ê‚é
					return 1;
				}
			}


			if (!rc_.PtInRect(pt3))
			{
				float_idx_ = -1;
				ClearSelect();
			}
			else 
			{
				ret = 1;
				SetSelectIndex(float_idx_);
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt3 = mat_.DPtoLP(lp);

			if ( md_ == 1 )
			{
				if ( rc_.PtInRect(pt3))
				{
					ret = 1;
					/*auto ls = dynamic_cast<D2DDropDownListbox*>(GetParentControl());
					if ( ls )
						ls->SetSelectIndex(float_idx_);*/
				}
				
			}			
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt3))
			{				
				FPointF pt = rc_.LeftTop();
				FRectF rc(pt.x, pt.y, FSizeF(rc_.Width()-BARWIDTH, 0));			
				int i = 0;

				rc.Offset(-scrollbar_off_.width, -scrollbar_off_.height);
				

				for( auto it = ar_.begin()+start_view_row_; it < ar_.end(); it++ )
				{
					auto rcitem = (*it)->GetRect();
					
					rc.bottom = rc.top + rcitem.Height();

					if ( rc.PtInRect(pt3))
					{						
						if ( float_idx_ != i )
						{
							float_idx_ = i;
							ret = 1;
							d->redraw();
							break;
						}
					}							

					rc.Offset(0,rcitem.Height());
					i++;
				}
				ret = 1;

				d->redraw();
			}			
		}
		break;


		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs ^args = (Windows::UI::Core::KeyEventArgs ^)lp;

			switch ( args->VirtualKey )
			{				
				case Windows::System::VirtualKey::Escape:
				{
					float_idx_ = -1;
				}
				break;
			}
		}
		break;
		

	}

	if ( ret == 0 && vbar_ )
		ret = vbar_->WndProc(d,message,wp,lp);
	else if ( ret == 0 )
		ret = DefWndProc(d,message,wp,lp);

	return ret;
}

void D2DListbox::AddItem(LPCWSTR str)
{
	auto pcxt = parent_->cxt();
	std::shared_ptr<ListboxItemString> item(new ListboxItemString(*pcxt,str));

	AddItem(item);
}

void D2DListbox::AddItem( std::shared_ptr<IListboxItem> item)
{	
	ar_.push_back(item);	

	items_height_ += item->GetRect().Size().height;

	if ( rc_.Height() < items_height_  )
	{
		if ( vbar_ == nullptr )
		{
			FRectF rc(0,0,BARWIDTH, rc_.Height());
			vbar_ = new D2DScrollbar();
			vbar_->Create( this, rc, VISIBLE,NONAME);
		}
		else
		{
			vbar_->SetTotalSize(items_height_);
			vbar_->Visible();
		}
	}
	else if ( vbar_ != nullptr )
	{
		vbar_->Hide();
	}
}
void D2DListbox::UpdateScrollbar(D2DScrollbar* bar )
{
	auto& info = bar->Info();
	
	if ( info.bVertical )
		scrollbar_off_.height = info.position / info.thumb_step_c;
	else
		scrollbar_off_.width = info.position / info.thumb_step_c;

	//if ( info.bVertical )
	//{
	//	float h = scrollbar_off_.height;
	//	start_view_row_ = 0;
	//	for( auto& it : ar_ )
	//	{
	//		h -= (it->GetRect().Height());

	//		if ( h < 0 )
	//			break;

	//		start_view_row_++;
	//	}
	//}
}

void D2DListbox::SetSelectIndex( int idx )
{		
	if ( -1 < idx && idx < (int)ar_.size() )
	{
		if ( typ_ == TYP::SINGLELINE )
		{
			ClearSelect();
		}
	
		ar_[idx]->SetSelect(true);
	}

}
void D2DListbox::ClearSelect()
{
	for( auto& it : ar_ )
		it->SetSelect(false);
}

void D2DListbox::Clear()
{
	ar_.clear();
	float_idx_ = -1;
	scrollbar_off_ = FSizeF(0,0);
	start_view_row_ = 0;

}

