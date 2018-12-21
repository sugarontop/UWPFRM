#include "pch.h"
#include "D2DUniversalControl.h"
#include "sybil.h"
#include "content/D2DWindowMessage.h"
#include "higgsjson.h"
using namespace V4;
using namespace sybil;

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

		

		case WM_D2D_LB_ADD_ITEMS_JSON:
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
		break;
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
		layout_->Release();
}
void ListboxItemString::SetText(ComPTR<IDWriteTextLayout> layout, const std::wstring& value)
{
	Clear();
	layout_ = layout;
	value_ = value;
}
//////////////////////////////
#define BARWIDTH 16

void D2DListbox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name,TYP typ, int local_id)
{
	
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);
	typ_ = typ;
	float_idx_ = -1;
	floadbr_ = CreateBrush( *parent_->cxt(), D2RGBA(233,0,233,100));
	items_height_ = 0;
	md_ = 0;
	start_view_row_ =0;
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
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			cxt.cxt->FillRectangle(rc_, cxt.white );

			FPointF pt = rc_.LeftTop();

			D2DRectFilter df(cxt, rc_);

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
						cxt.cxt->FillRectangle(rc, cxt.halftoneRed );	
					}
					else if ( k == float_idx_ )
					{
						FRectF rc(0,0,rc_.Width(), hi );
						cxt.cxt->FillRectangle(rc, floadbr_ );	
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

			if ( args->VirtualKey == Windows::System::VirtualKey::Control )
			{
				

				ret = 1;				
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
	if ( idx < (int)ar_.size() )
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

////////////////////////////////////////////////////////////////////////////////////////
D2DCombobox::D2DCombobox()
{

}
void D2DCombobox::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);
	Clear();
	ListboxHeight_ = 300;
	md_ = 0;
	floating_relative_idx_ = -1;
	start_idx_ = 0;
	
}
int D2DCombobox::AddItem( LPCWSTR key, LPCWSTR value )
{
	KeyValue x;
	x.key = key;
	x.value = value;
	items_.push_back(x);

	return items_.size();
}
void D2DCombobox::Clear()
{
	items_.clear();
	select_idx_ = -1;
}
void D2DCombobox::OnReleaseCapture(int layer)
{
	D2DControl::OnReleaseCapture(layer);
}

int D2DCombobox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && !IsImportantMsg(message) ) 
		return 0;

	int ret = 0;
	bool bl = false;
	
	switch( message )
	{
		case WM_PAINT:
		{
			DrawSelectedText(d);

			if ( IsCaptured())
				DrawListbox(d);
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);


			FRectF rclistbox = rc_;
			rclistbox.top = rc_.bottom;
			rclistbox.bottom = rclistbox.top + ListboxHeight_;

			if ( rc_.PtInRect(pt ))
			{
				parent_control_->SetCapture(this);

				d->redraw();
				ret = 1;
				floating_relative_idx_ = -1;

				md_ = 1; // show listbox
				ret = 1;
			}
			else if ( rclistbox.PtInRect(pt))
			{
				md_ = 2; // select in listbox
				ret = 1;
			}
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( md_ == 1 )
			{
				FPointF pt = mat_.DPtoLP(lp);

				FRectF rclistbox = rc_;
				rclistbox.top = rc_.bottom;
				rclistbox.bottom = rclistbox.top + ListboxHeight_;

				int new_floating_idx = floating_relative_idx_;
				if ( rclistbox.PtInRect(pt))
				{										
					new_floating_idx = (int)((pt.y - rclistbox.top) / 30.0f);
								
				}

				if ( new_floating_idx != floating_relative_idx_ )
				{
					d->redraw();	
					floating_relative_idx_ = new_floating_idx;
				}

				ret = 1;
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( md_ == 2 && this == parent_control_->GetCapture())
			{								
				parent_control_->ReleaseCapture();
				d->redraw();
				ret = 1;

				
				_ASSERT( (stat_ & STAT::CAPTURED) == 0 );

				select_idx_ = floating_relative_idx_ + start_idx_;
				OnSelectChanged();
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			if ( parent_control_->GetCapture() == this  )
			{
				switch( arg->VirtualKey )
				{
					case Windows::System::VirtualKey::Escape:
					{						
						parent_control_->ReleaseCapture();
						d->redraw();
						ret = 1;
					}
				}				
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			if ( md_ == 1 && this == parent_control_->GetCapture())
			{
				Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;

				int delta = arg->CurrentPoint->Properties->MouseWheelDelta;
				ret = 1;
				d->redraw();
				start_idx_ = max(0,min((int)items_.size()-1, (delta < 0 ? (int)start_idx_+1 : (int)start_idx_-1 )));

				OnSelectChanged();
			}
		}
		break;
	}


	return ret;
}
void D2DCombobox::DrawSelectedText(D2DWindow* d)
{
	auto& cxt = *(d->cxt());
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();
	mat.Offset(rcb.left, rcb.top);

	auto rc1 = rcb.ZeroRect();
	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );

	std::wstring title;

	if ( select_idx_ == -1 )
		title = L"please select";
	else if ( 0 <= select_idx_ && select_idx_ < (int)items_.size())
	{
		title = items_[select_idx_].value;
	}

	sybil::DrawTextCenter( cxt.cxt, rc1, title.c_str(), title.length(),cxt.textformat, cxt.black  ); 

	mat.PopTransform();

}
void D2DCombobox::DrawListbox(D2DWindow* d)
{
	auto& cxt = *(d->cxt());
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();


	mat.Offset(rcb.left, rcb.top);

	auto rc1 = rcb.ZeroRect();
	


	rc1.Offset( 0, rc1.Height());
	rc1.bottom = rc1.top + ListboxHeight_;

	D2DRectFilter fil( cxt, rc1.GenInflateRect(1,1) );

	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );


	rc1.bottom = rc1.top + 30;
	int ridx = 0;
	for( UINT i = start_idx_; i < items_.size(); i++ )
	{
		if ( floating_relative_idx_ == ridx )
		{
			cxt.cxt->FillRectangle( rc1, cxt.halftone );
		}
		auto& it = items_[i];
		sybil::DrawTextCenter( cxt.cxt, rc1, it.value.c_str(), it.value.length(),cxt.textformat, cxt.black  ); 
		rc1.Offset( 0, 30 );
		ridx++;
	}


	mat.PopTransform();

}
void D2DCombobox::OnSelectChanged()
{
	wparam wp;
	wp.cmb = this;

	if ( OnSelectChanged_ )
		OnSelectChanged_(this);
	else
		parent_->SendMessage( WM_D2D_COMBOBOX_CHANGED, (INT_PTR)&wp, nullptr );

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//D2DTest::D2DTest()
//{
//	bfresize_ = false;
//	cc_ = nullptr;
//	mode_ = 0;
//}
//void D2DTest::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
//{
//	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
//
//	minmum_v_ = [this](D2DTest*sender, D2DContext& cxt) {
//		
//		cxt.cxt->FillRectangle( rc_, cxt.white );
//		cxt.cxt->DrawText( L"minum", 5, cxt.cxtt.textformat, rc_, cxt.black );
//
//	};
//	default_v_ = [this](D2DTest*sender, D2DContext& cxt) {
//		
//		ComPTR<ID2D1SolidColorBrush> br;
//		cxt.cxt->CreateSolidColorBrush( D2RGBA(200,200,200,100), &br );
//
//		cxt.cxt->FillRectangle( rc_, br );
//	};
//
//
//	D2DButton* b1 = new D2DButton();
//	b1->CreateButton( parent, this, FRectF(0,10,100,30),VISIBLE,L"a1", L"noname");
//	b1->OnClick_ = [this](D2DButton* b)
//	{
//
//		this->dstRect_ = FRectF( 1500,0,FSizeF(0,1200));
//		this->bfresize_ = true;
//
//	};
//
//
//	FRectF rc3( 100,100, FSizeF(100, 30));
//	 b1 = new D2DButton();
//	b1->CreateButton(parent,this, rc3,VISIBLE,L"a1",L"noname" );	
//	b1->OnClick_ = [parent, this](D2DButton*b)
//	{
//		D2DMessageBox d;
//		d.Show(parent, this, FRectF(100,100,FSizeF(500,200)), L"test", L"messagebox" );
//	};
//
//
//}
//int D2DTest::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
//{
//	int ret = 0;
//	if ( !(stat_ & VISIBLE) )
//		return ret;
//	switch( message )
//	{
//
//		case WM_PAINT:
//		{
//			auto& cxt = *(d->cxt());
//
//			D2DMatrix mat(cxt);
//			mat_ = mat.PushTransform();
//
//			FRectF rcb = rc_.GetBorderRect();
//			//mat.Offset(rcb.left, rcb.top); // border rectŠî€
//						
//						
//			FRectF rca = rc_.GetContentBorderBase(); 
//
//			
//
//			if ( bfresize_ || cc_ !=nullptr )
//			{
//				FRectF rcz = dstRect_;
//				FRectF rc;
//				cc_ = DrawDriftRect( cc_, &rc, cxt.tickcount_, rc_, rcz, 500 );
//				if ( cc_ == nullptr )
//				{					
//					bfresize_ = false;
//					mode_ = ( dstRect_.Size().width == 0 ? 0 : 1);
//
//					if ( mode_ == 0 )
//					{
//						if ( ParentControl()->GetCapture() == this )
//							this->ParentControl()->ReleaseCapture();
//					}
//				}
//
//				rc_ = rc;
//
//				default_v_(this, cxt );
//
//				//cxt.cxt->DrawRectangle( rc, cxt.black );
//				d->redraw();
//
//			}
//			else			
//			{
//				if ( mode_ == 0 )
//					minmum_v_(this, cxt );
//				else
//					default_v_(this, cxt );
//
//				FRectF rcb = rc_.GetBorderRect();
//				mat.Offset(rcb.left, rcb.top);
//
//				if ( rcb.Size().width > 0 )
//					D2DControls::DefWndProc(d,message,wp,lp);
//
//				
//			}
//
//			mat.PopTransform();
//			return 0;
//		}
//		break; 
//		case WM_MOUSEMOVE:
//		{
//			/*if ( rc_.Size().width <= 0 )
//			{
//				bool bl = ( this->ParentControl()->GetCapture() == this );
//
//				if ( bl )
//					this->ParentControl()->ReleaseCapture();
//			}*/
//
//			ret = D2DControls::DefWndProc(d,message,wp,lp);
//		}
//
//		break;
//		default:
//			ret = D2DControls::DefWndProc(d,message,wp,lp);
//		break;
//	}
//	if ( ret == 0 )
//		ret = ( this->parent_control_->GetCapture() == this ? 1 : 0 );
//
//	return ret;
//}
//




