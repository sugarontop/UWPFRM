#include "pch.h"
#include "D2DWindowMessage.h"
#include "D2DDevelop.h"
#include "D2DTextbox.h"

#include "higgsjson.h"


using namespace HiggsJson;

namespace V4 {

int D2DList::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			FRectF rcborder = rc_.GetBorderRect();

			cxt.cxt->FillRectangle( rcborder, cxt.bluegray );

			mat.Offset( rcborder.left, rcborder.top );		
			
			
			DrawList( cxt );
			

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_PROPERTY_LIST_JSON :
		{
			D2DJson* info = (D2DJson*)wp;
			D2DControl* crl = (D2DControl*)info->sender;
			BSTR json = info->json;

			Parse(json);

			d->redraw();
			ret = 1;
		}
		break;
	}


	return ret;

}
void D2DList::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	Item m;
	m.title = L"D2DList";
	m.value = CStrHex( (DWORD)this);
	m.typ = 0;
	ar_.push_back(m);

}
void D2DList::Parse(BSTR json)
{
	std::vector<Higgs> ar;
	ParseList(json, ar );

	//まだ途中


}
void D2DList::DrawList(D2DContext& cxt )
{
	FRectF rct(0,0,200,26 );
	FRectF rcv(200,0,400,26 );

	for( auto& it : ar_ )
	{
		cxt.cxt->DrawText( it.title.c_str(), it.title.length(), cxt.textformat, rct, cxt.white );

		cxt.cxt->DrawText( it.value.c_str(), it.value.length(), cxt.textformat, rcv, cxt.white );

		FPointF pt1,pt2;
		pt1.y = pt2.y = rct.bottom;
		pt2.x = rcv.right;
		cxt.cxt->DrawLine( pt1, pt2, cxt.white );

		rct.Offset( 0, 30 );
		rcv.Offset( 0, 30 );

	}


}

////////////////////////////////////////////////////////////////////////////////////////

#define COLUM_TITLE_WIDTH 100.0f
#define CELLOFFX 2
#define CELLOFFY 0

void D2DPropertyControls::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	
	//for( int i =0; i < 4; i++)
	//{
	//	Row r;
	//	r.readonly = true;
	//	r.title = L"(名前)";
	//	r.value = L"test.cpp";
	//	r.typ = TEXT;

	//	if ( i==2 || i == 1)
	//		r.readonly = false;
	//	if ( i==3 )
	//	{
	//		r.typ = LIST;
	//		r.readonly = false;
	//	}
	//	
	//	ar_.push_back(r);
	//}

	PreDraw();

	FRectFBoxModel txrc(0,0,100,20);

	txrc.Padding_.l = CELLOFFX+2;
	txrc.Padding_.t = CELLOFFY;

	auto& caret = Caret::GetCaret();
	D2CoreTextBridge*  ime_bridge =  dynamic_cast<D2DMainWindow*>(pacontrol->GetParentWindow())->GetImeBridge();
	tbox_ = new D2DTextbox(*ime_bridge, caret);
	tbox_->Create(this, txrc, 0, NONAME);


	ls_ = new D2DDropDownListbox();
	ls_->Create( this, txrc, 0,NONAME );


	//WParameterString wp;
	//wp.str1 = SysAllocString(L"1.0");
	//wp.str2 = SysAllocString(L"key2");
	//ls_->WndProc(parent_, WM_D2D_LB_ADDITEM, (INT_PTR)&wp, nullptr);
	//wp.str1 = SysAllocString(L"2.0");
	//wp.str2 = SysAllocString(L"key3");
	//ls_->WndProc(parent_, WM_D2D_LB_ADDITEM, (INT_PTR)&wp, nullptr);

	//ls_->WndProc( parent_, WM_D2D_LB_SET_SELECT_IDX, (INT_PTR)0, nullptr);
}

void D2DPropertyControls::Load(const BSTR json )
{
	ar_.clear();
	data_ = ::SysAllocString(json);
	std::vector<Higgs> ar;
	HiggsJson::ParseList(data_, ar);
	
	std::vector<std::map<std::wstring,Higgs>> mm;
	for( auto& it : ar )
	{
		std::map<std::wstring,Higgs> m;
		ParseMap(it.head, m );
		mm.push_back(m);
		
		int no = ToInt(m[L"no"]);
		auto typ = ToStr(m[L"typ"]);
		auto title = ToStr(m[L"title"]);
		auto value = ToStr(m[L"value"]);
		auto readonly = ToBool(m[L"readonly"]);
		Higgs items = m[L"items"];

		D2DPropertyControls::Row r;
		r.title = title;
		r.value = value;
		r.readonly = readonly;
		r.typ = TYP::TEXT;
		r.typ = (L"listbox"==typ? TYP::LIST : r.typ );
		r.items = items;
		r.selectidx = 0;

		
		ar_.push_back(r);
	}

	PreDraw();
}




int D2DPropertyControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			FRectF rcborder = rc_.GetBorderRect();

			//cxt.cxt->FillRectangle( rcborder, cxt.bluegray );

			mat.Offset( rcborder.left, rcborder.top );		
			
			
			DrawList( cxt );
			
			
			DefPaintWndProc(d,message,wp,lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			select_row_ = -1;
			int j = 0;
			if ( rc_.PtInRect(pt3))
			{
				ReleaseCapture();
				tbox_->Hide();
				ls_->Hide();

				for(auto& it : ar_ )
				{
					if ( !it.readonly )
					{
						auto rc = it.rc;
						pt3 = matd_.DPtoLP(lp);

						if (rc.PtInRect(pt3) && it.typ == TEXT)
						{
							select_row_ = j;	
							rc.Offset(COLUM_TITLE_WIDTH,0);
							if ( tbox_->IsHide())
							{
								tbox_->Visible();
								
								FRectFBoxModel txrc = rc;
								txrc.Padding_.l = CELLOFFX;
								txrc.Padding_.t = CELLOFFY;
								tbox_->SetRect(txrc);
								tbox_->SetText(it.value.c_str());

								tbox_->Activate(); // 内部でSetCaptureされる

								ret = 1;
								break;
							}							
						}
						else if (rc.PtInRect(pt3) && it.typ == LIST)
						{
							select_row_ = j;	
							rc.Offset(COLUM_TITLE_WIDTH,0);
							if ( ls_->IsHide())
							{
								FRectFBoxModel txrc = rc;
								txrc.Padding_.l = CELLOFFX;
								txrc.Padding_.t = CELLOFFY;

								if ( it.items.head )
								{
									ls_->WndProc(parent_, WM_D2D_LB_CLEAR, (INT_PTR)0, nullptr);
									ls_->WndProc(parent_, WM_D2D_LB_ADD_ITEMS_JSON, (INT_PTR)it.items.head, nullptr);
									ls_->WndProc(parent_, WM_D2D_LB_SET_SELECT_IDX, (INT_PTR)it.selectidx, nullptr);
								}

								ls_->Visible();
								ls_->SetRect(txrc);
								SetCapture(ls_);
								ret = 1;
								break;
							}
						}
					}
					j++;
				}
											
			}

		}
		break;
		case WM_KEYDOWN:
		{	
			if ( !tbox_->IsCaptured())
			{			
				Windows::UI::Core::KeyEventArgs ^args = (Windows::UI::Core::KeyEventArgs ^)lp;
				if ( args->VirtualKey == Windows::System::VirtualKey::Escape )
				{					
					ReleaseCapture();					
					tbox_->Hide();
					ls_->Hide();
				}
				else if ( args->VirtualKey == Windows::System::VirtualKey::Enter)
				{
					if (tbox_->IsHide())
					{
						//auto rc = sel_;
						//rc.y = min(sel_.y+1, row_limit());
						//SetSelectCell(rc);
					}
				}
				ret = 1;

			}
		}
		break;
		case WM_D2D_TEXTBOX_CHANGED:
		{
			WParameter* wps = (WParameter*)wp;
			if (wps->sender = tbox_ )
			{
				LPCWSTR s = (LPCWSTR)wps->prm;
				
				if ( -1 < select_row_ && select_row_ < ar_.size() )
				{
					ar_[select_row_].value = s;


					PreDraw();

					ReleaseCapture();
					tbox_->Hide();
				}

				ret = 1;
			}


		}
		break;
		case WM_D2D_ESCAPE_FROM_CAPTURED:
		{
			WParameter* wps = (WParameter*)wp;
			if ( tbox_ == wps->sender )
			{
				tbox_->Hide();
				d->redraw();
				ret = 1;
			}
			else if ( ls_ == wps->sender )
			{
				ls_->Hide();
				d->redraw();
				ret = 1;
			}

		}
		break;
		case WM_D2D_LB_EVNT_SELECT_CHANGE:
		{
			WParameter* wps = (WParameter*)wp;
			if ( wps->sender == ls_ )
			{
				int idx = wps->no; // listbox内のセレクト行
				
				// select_row_:D2DPropertyControlsの上からの行
				ar_[select_row_].value = ls_->Value(idx);
				ar_[select_row_].selectidx = idx;

				PreDraw();
				ret = 1;				
			}

		}
		break;



	}


	if ( ret == 0 )
		ret = DefWndProc(d,message,wp,lp);

	return ret;
}

void D2DPropertyControls::PreDraw()
{	
	D2DContext& cxt = *(parent_->cxt());

	const float height =  20.0f;
	FRectF rc1(0,0,FSizeF(200,height));


	for(auto& it : ar_ )
	{
		if ( it.ctitle ) 
			it.ctitle.Release();
		if ( it.cvalue ) 
			it.cvalue.Release();

		auto sz1 = CreateTextLayout(cxt, it.title.c_str(), it.title.length(), &it.ctitle);
		auto sz2 = CreateTextLayout(cxt, it.value.c_str(), it.value.length(), &it.cvalue);

		it.rc = rc1;
		rc1.Offset(0, height);
	}

}

void D2DPropertyControls::DrawList(D2DContext& cxt)
{
	D2DMatrix mat(cxt);	
	mat.PushTransform();

	FPointF pt;
	const float height =  20.0f;
	FRectF rc1(0,0,FSizeF(200,height));

	for(auto& it : ar_ )
	{
		auto txclr = (it.readonly ? cxt.bluegray : cxt.black);		
		cxt.cxt->DrawRectangle( it.rc, cxt.bluegray );		
		cxt.cxt->FillRectangle( it.rc, cxt.white );		
		cxt.cxt->DrawTextLayout(it.rc.LT(CELLOFFX,CELLOFFY), it.ctitle, txclr);
	}
	
	mat.Offset(COLUM_TITLE_WIDTH,0);

	matd_ = mat;
	
	for(auto& it : ar_ )
	{
		auto txclr = (it.readonly ? cxt.bluegray : cxt.black);		
		cxt.cxt->DrawRectangle( it.rc, cxt.bluegray );		
		cxt.cxt->FillRectangle( it.rc, cxt.white );		
		cxt.cxt->DrawTextLayout(it.rc.LT(CELLOFFX,CELLOFFY), it.cvalue, txclr);		
	}
	mat.PopTransform();
}












};