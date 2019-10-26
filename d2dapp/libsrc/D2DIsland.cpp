#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DIsland.h"

using namespace V4;


void DrawText::Text(D2DContext& cxt, LPCWSTR str)
{
	t_.Release();
	cxt.wfactory->CreateTextLayout(str, wcslen(str), cxt.textformat, 1000, 1000, &t_);
}
void DrawText::d(D2DContext& cxt, const FPointF& pt, ID2D1Brush* br)
{
	cxt.cxt->DrawTextLayout(pt,t_,br);
}




D2DIsland::D2DIsland()
{
	

}
void D2DIsland::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	mode_ = MODE::NORMAL;

	InnerCreateWindow(pacontrol, rc, stat, name, local_id);
	

	auto cxt = GetParentWindow()->cxt();
	title_.Text(*cxt, name);
	clridx_ = 0;
}


int D2DIsland::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (mode_ == MODE::NORMAL)
		return WndProcN(d, message, wp, lp);
	else
		return WndProcB(d, message, wp, lp);
}





int D2DIsland::WndProcN(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			FRectF rc(rc_);

			ComPTR<ID2D1SolidColorBrush> clr[] = { cxt.blue, cxt.red, cxt.gray };
			
			
			cxt.cxt->DrawRectangle(rc, clr[clridx_]);
						

			mat.Offset( rc_.left, rc_.top );

			FRectF rct(20,rc_.Size().height-30,FSizeF(rc_.Size().width, 30));
			//cxt.cxt->DrawText(name_.c_str(),name_.length(),cxt.textformat,rct,cxt.black);
			title_.d(cxt,rct.LeftTop(), cxt.black);


			DefPaintWndProc(d, message, wp, lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONUP:
		{
			LOGPT(pt3,wp)

			if (!rc_.PtInRect(pt3) && this == GetCapture())
			{
				ReleaseCapture(this);
				ret = 1;
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
			LOGPT(pt3,wp)

			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
		}
		break;
		case WM_D2D_USERCD + 2:
		{
			int id = wp;

			clridx_ = id-10;

		}
		break;
		
	}

	if ( ret == 0 )
		ret = D2DControls::DefWndProc(d,message,wp,lp);

	return ret;

}
void D2DIsland::ModeChange()
{
	mode_ = (mode_ == MODE::NORMAL ? MODE::MINIBAR : MODE::NORMAL);
}
int D2DIsland::WndProcB(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	if (IsHide() && !IsImportantMsg(message))
		return 0;

	switch (message)
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

		
			

			ComPTR<ID2D1SolidColorBrush> br;
			cxt.cxt->CreateSolidColorBrush(D2RGB(220, 220, 220), &br);
			
			cxt.cxt->FillRectangle(rcMin_, br );

			mat.Offset(rcMin_.left, rcMin_.top);

			FRectF rct(10, 0, FSizeF(rcMin_.Size().width, 30));			
			title_.d(cxt, rct.LeftTop(), cxt.black);
			
			mat.PopTransform();
			
			return 0;
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			LOGPT(pt3,wp);

			if (rcMin_.PtInRect(pt3))
			{
				mode_ = (mode_ == MODE::NORMAL ? MODE::MINIBAR : MODE::NORMAL);
				ret = 1;

				
			}			
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp)

			if (rcMin_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
		}
		break;
		case WM_RBUTTONDOWN:
		{
			LOGPT(pt3, wp);

			if (rcMin_.PtInRect(pt3))
			{
				ret = 1;


				std::vector<MenuItem> items;

				MenuItem m;
				m.message_id = WM_D2D_USERCD+1;
				m.viewnm = _u("test"); items.push_back(m);
				m.viewnm = _u("test1"); items.push_back(m);
				m.viewnm = _u("test2"); items.push_back(m);

				RightButtonFloatMenu(pt3, this, items, D2RGB(0,255,255));
			}
		}
		break;
		case WM_D2D_USERCD + 1:
		{
			if ( (INT_PTR)this == wp )
			{


				ret = 1;

			}

		}
		break;
		
	}

	//if (ret == 0)
	//	ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;
}



class SqueezeOne
{
	public :
		SqueezeOne():fn_(liner_trajectory){}
		SqueezeOne(std::function<FRectF * (FRectF, FRectF, int)> fn):fn_(fn){}

		static FRectF* liner_trajectory(FRectF frc, FRectF trc, int stepcount )
		{
			FRectF* r = new FRectF[stepcount];

			for(int i = 0; i < stepcount; i++ )
			{
				FRectF& rk = r[i];

				rk.left = frc.left + i*(trc.left - frc.left) / stepcount;
				rk.top = frc.top + i * (trc.top - frc.top) / stepcount;
				rk.right = frc.right + i * (trc.right - frc.right) / stepcount;
				rk.bottom = frc.bottom + i * (trc.bottom - frc.bottom) / stepcount;
			}
			r[stepcount -1] = trc;
			
			return r;
		}


		static DWORD CALLBACK anime(LPVOID p)
		{
			SqueezeOne* ps = (SqueezeOne*)p;

			for (int i = 0; i < ps->stepcount_; i++)
			{
				int ii = ps->step_++;
				for( auto& it : ps->sqrects_ )
				{
					FRectF rc = (*it).prc_[ii];

					*((*it).target) = rc;
				}

				ps->win_->InvalidateRect();
				Sleep(16);
			}

			delete ps; // `æI¹
			return 0;
		}

		
		void Start(D2DWindow* pw, const std::vector<std::shared_ptr<RectSqueeze>>& rc, int stepcount)
		{
			// OÕðZo
			for (auto& it : rc)
			{
				(*it).prc_ = fn_((*it).frc, (*it).trc, stepcount);
				*(*it).target = (*it).frc;
			}

			step_ = 0;
			stepcount_ = stepcount;
			win_ = pw;
			sqrects_ = rc;



			DWORD dw;
			CreateLightThread(anime, this, &dw);
		}
	protected :		
		int step_, stepcount_;
		D2DWindow* win_;
		FRectF* target_;

		std::vector<std::shared_ptr<RectSqueeze>> sqrects_;
		std::function<FRectF * (FRectF, FRectF, int)> fn_;
};

void SoftSqueeze(D2DWindow* p, const std::vector<std::shared_ptr<RectSqueeze>>& ar, int milisec, int typ)
{
	// 60fps
	int cnt = (int)(milisec / 60.0f);

	if ( typ == 0 )
	{
		SqueezeOne* so = new SqueezeOne();
		so->Start(p, ar, cnt);
	}
	else  if ( typ == 1 )
	{
		std::function<FRectF*(FRectF, FRectF, int)> fn;

		fn = [](FRectF frc, FRectF trc, int stepcount)->FRectF*
		{
			// ¼ñ]ãÉtrcÖÚ®·éOÕ

			FRectF* r = new FRectF[stepcount];
			
			float c = (frc.left + trc.left)/2;
			float hankei = abs(frc.left-c);

			const int half = stepcount/2;
			

			FPointF* p = new FPointF[half];
			const float pai2 = 2*3.14159f;
			for (int i = 0; i < half; i++)
			{
				float rad = (pai2 - 3.14159f / (half-1) * i);

				p[i].y = hankei*sin(rad);
				p[i].x = hankei*cos(rad);
			}

			int i = 0;
			for (i = 0; i < half; i++)
			{
				FRectF& rk = r[i];
				rk = frc;
				rk = rk.Offset(p[i].x, -p[i].y );
			}

			frc = r[i-1]; 

			for ( int j =0 ; j < half; j++)
			{
				FRectF& rk = r[j + half];

				rk.left = frc.left + j * (trc.left - frc.left) / half;
				rk.top = frc.top + j * (trc.top - frc.top) / half;
				rk.right = frc.right + j * (trc.right - frc.right) / half;
				rk.bottom = frc.bottom + j * (trc.bottom - frc.bottom) / half;

			}
			r[stepcount - 1] = trc;

			delete [] p;
			return r;
		};

		SqueezeOne* so = new SqueezeOne(fn);
		so->Start(p, ar, cnt);
	}


}
///////////////////////////////////////////////////////////////////////////////


void D2DLery::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);


	FRectF rc1(0, 30, 100, 60);

	WCHAR* nm[] = { L"BLUE", L"RED", L"GRAY" };
	for (int i = 0; i < 3; i++)
	{
		D2DLeryRadioButton* p1 = new D2DLeryRadioButton(idx_);
		p1->Create(this, rc1, 0, nm[i], 10+i );
		rc1.Offset(0, 35);
	}

}
int D2DLery::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			FRectF rc(0,0,100,30);
			cxt.cxt->DrawText(L"¸§", 2, cxt.textformat, rc, cxt.black );


			D2DControls::DefPaintWndProc(d, message, wp, lp);

			mat.PopTransform();
			return 0;
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
		case WM_LBUTTONDOWN:
		{
			LOGPT(pt3, wp);

			FRectF rc(rc_);
			rc.SetHeight(30);

			if (rc.PtInRect(pt3))
			{
				if (controls_[0]->IsVisible())
				{
					Squeeze(false);
				}
				else
				{
					for (auto& it : controls_)
						it->Visible();

					Squeeze(true);
				}

				ret = 1;
			}

		}
		break;
		case WM_D2D_USERCD + 1:
		{
			D2DLeryRadioButton* rb = (D2DLeryRadioButton*)wp;
			int id = rb->GetId();

			idx_ = id;
			parent_control_->WndProc(d, WM_D2D_USERCD + 2, id, nullptr);

			ret = 1;

		
		}
		break;


	}


	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;
}



DWORD CALLBACK D2DLery::anime1(LPVOID p)
{
	D2DLery* ler = (D2DLery*)p;
	auto w = ler->GetParentWindow();
	int k = 0;
	
	for (auto& it : ler->controls_)
	{		
		auto rc2 = ler->target_[k];
		FRectF xrc = rc2.frc;

		float offcx = (rc2.trc.left - rc2.frc.left)/10;

		it->SetRect(rc2.frc);
		for(int i = 0; i < 10; i++ )
		{
			it->SetRect(xrc);

			w->InvalidateRect();

			xrc.Offset(offcx,0);
			Sleep(16);
		}

		it->SetRect(rc2.trc);

		k++;
	}

	bool hide = (ler->target_[0].trc.left < 0);

	k = 0;
	for (auto& it : ler->controls_)
	{
		it->SetRect(ler->target_[k++].lastrc);

		if ( hide )
			it->Hide();
		
	}

	delete [] ler->target_;
	ler->target_ = nullptr;
	return 0;
}

void D2DLery::Squeeze(bool isvisible)
{
	target_ = new FRectF2[controls_.size()];
	int i = 0;

	if (isvisible)
	{
		for(auto& it : controls_)
		{
			auto rc = it->GetRect();
			target_[i].trc = rc;
			target_[i].lastrc = rc;
			rc.Offset( -rc.Width(), 0 );
			target_[i].frc = rc;
			it->SetRect(rc);
			i++;
		}
	}
	else
	{
		for (auto& it : controls_)
		{
			auto rc = it->GetRect();
			target_[i].lastrc = rc;
			target_[i].frc = rc;
			rc.Offset(-rc.Width(), 0);
			target_[i].trc = rc;
			//it->SetRect(rc);

			
			i++;
		}
	}

	DWORD dw;
	CreateThread(0, 0, anime1, this, 0, &dw);
}

void D2DLeryRadioButton::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

	checked_ = false;
}


int D2DLeryRadioButton::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			FRectF rc(0, 0, 100, 30);
			
			D2D1_ELLIPSE el;
			el.point= FPointF(15,15);
			el.radiusX = el.radiusY = 5;

			cxt.cxt->DrawEllipse(el, cxt.black);
			
			if (idx_ == this->GetId())
				cxt.cxt->FillEllipse(el, cxt.black);

			rc.Offset(30, 5);
			cxt.cxt->DrawText(name_.c_str(), name_.length(), cxt.textformat, rc, cxt.black);


			mat.PopTransform();
			return 0;
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
		case WM_LBUTTONDOWN:
		{
			LOGPT(pt3, wp);

			if (rc_.PtInRect(pt3))
			{
				idx_ = this->GetId();

				
				{
					parent_control_->WndProc(d, WM_D2D_USERCD+1, (INT_PTR)this, nullptr); 
				}

				ret = 1;
			}
		}
		break;
		case WM_D2D_USERCD+1:
		{

		}
		break;

	}


	return ret;
}

///////////////////////////////////////////////////

FRectF off_rect(const FRectF& rc1, float cx, float cy )
{
	FRectF rc(rc1);
	rc.Offset(cx,cy);
	return rc;
}

#define ROWHEIGHT 24

D2DSliderButton::D2DSliderButton()
{
	MenuItem mi;
	mi.message_id = -1;
	mi.viewnm= _u("not define");
	isVertical_ =  true;
	
	items_.push_back(mi);
	float_idx_ = -1;
	target_ = nullptr;
}
void D2DSliderButton::Set( D2DControl* target, const std::vector<MenuItem>& items)
{
	_ASSERT(target);

	items_ = items;
	isVertical_ = true;
	float_idx_ = -1;
	target_ = target;

	float fontheight = ROWHEIGHT;

	rc_.SetHeight(fontheight *items.size());


}

int D2DSliderButton::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() && !IsImportantMsg(message) && message != WM_D2D_THREAD_COMPLETE)
		return 0;

	if (isVertical_)
	{
		return WndProcA(d, message, wp, lp);

	}
	else
	{
		return WndProcB(d,message, wp,lp);
	}
}
int D2DSliderButton::WndProcA(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	// vertical float menu control
	int ret = 0;

	switch (message)
	{
		case WM_PAINT:
		{
			CXTM(d)
			mat_ = mat.PushTransform();

			auto rc(rc_);
			rc.Offset(3,3);
			FillRectangle(cxt, rc,cxt.bluegray); // draw shadow

			D2DRectFilter rf(cxt, rcFilter_);
			mat.Offset(rc_.left, rc_.top);

			auto br = CreateBrush(cxt, D2RGB(255,245,0));
			FillRectangle(cxt, rc_.ZeroRect(), br );

			float rowh = ROWHEIGHT;
			rc.SetRect(2,2, rc_.Size().width-2, rowh);
			int i = 0;
			for( auto& it : items_ )
			{
				if (float_idx_ == i++)
				{
					auto br2 = CreateBrush(cxt, D2RGBA(153, 217, 234,240));
					FillRectangle(cxt, rc, br2);
				}
				
				cxt.cxt->DrawText( it.viewnm.c_str(), it.viewnm.length(), cxt.textformat, rc, cxt.black );
				rc.Offset(0, rowh);
			}


			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			LOGPT(pt3, wp);
			Close();

			if (rc_.PtInRect(pt3))
			{				
				ret = 1;

				if ( -1 <float_idx_ && float_idx_ <items_.size())
				{
					int msg = items_[float_idx_].message_id;
					if ( msg > 0 )
					{
						GetParentWindow()->PostMessage( msg, (INT_PTR)target_, nullptr);

					}
				}
			}
			else if (float_idx_ > 0)
			{
				
				ret = 1;
			}
		}
		break;		
		case WM_MOUSEMOVE:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				float_idx_ = ((int)(pt3.y-rc_.top))/ ROWHEIGHT;
				ret =1;
				d->redraw();
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch (arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:

				if (GetParentControl()->GetCapture() == this)
				{							
					
					Close();
					
					ret = 1;
				}
				break;
			}

		}
		break;
	}

	return ret;

}
void D2DSliderButton::Close()
{
	if (GetParentControl()->GetCapture() == this)
		GetParentControl()->ReleaseCapture();		
	DestroyControl();
}
int D2DSliderButton::WndProcB(D2DWindow * d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs ^ lp)
{
	int ret = 0;
	switch (message)
	{
		case WM_PAINT:
		{
			CXTM(d)
			mat_ = mat.PushTransform();

			
			D2DRectFilter rf(cxt, rcFilter_);
			mat.Offset(rc_.left, rc_.top);
			
			for(int i = 0; i < btncnt_; i++)
			{
				cxt.cxt->DrawRectangle(btn_[i], cxt.black);
				cxt.cxt->FillRectangle(btn_[i], cxt.bluegray);

				WCHAR s[2];s[1]=0;
				s[0]='1'+i;

				cxt.cxt->DrawText(s,1,cxt.textformat, off_rect(btn_[i],5,5), cxt.white);
			}


			

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}			
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if (rc_.PtInRect(pt3))
			{
				DrawSqueeze();
				ret = 1;
			}
			
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch (arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:
					
					if (GetParentControl()->GetCapture() == this )
					{
						DrawSqueeze();
						ret = 1;
					}
					else
					{
						if (IsVisible())
						{
							DrawSqueeze();
						}
					}
				break;
			}

		}
		break;
		case WM_D2D_THREAD_COMPLETE:
		{
			if ( (INT_PTR)this == wp)
			{
				if ( isModal_ )
				{				
					if ( IsHide() )
					{
						GetParentControl()->ReleaseCapture();
					}
					else
					{
						GetParentControl()->SetCapture(this);
					}					
				}				
				ret = 1;
			}

		}
		break;

	}

	return ret;
}
void D2DSliderButton::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

	btn_ = std::shared_ptr<FRectF[]>( new FRectF[btncnt_] );

	FRectF rc1;

	rc1.SetHeight(rc.Height());
	rc1.SetWidth(rc.Width()/btncnt_);

	for( int i = 0; i < btncnt_; i++ )
	{
		btn_[i] = rc1;
		rc1.Offset(rc1.Width(),0);
	}

	rcFilter_ = rc;

}
static DWORD CALLBACK _anime(LPVOID p)
{
	FRectF3* sb = (FRectF3*)p;
	D2DSliderButton* psb = (D2DSliderButton*)sb->obj;

	float step = (sb->trc.left - sb->frc.left)/10.0f;

	
	FRectF rc1 = sb->frc;
	for(int i=0; i < 10; i++)
	{
		psb->SetRect(rc1);

		if ( i== 0)
			psb->Visible();

		rc1.Offset(step,0);

		psb->GetParentWindow()->InvalidateRect();
		Sleep(16);
	}


	psb->SetRect(sb->trc);

	if ( sb->trc.left < 0 )
		psb->Hide();

	delete sb;

	psb->GetParentWindow()->PostMessage(WM_D2D_THREAD_COMPLETE, (INT_PTR)psb, nullptr);

	return 0;
};

void D2DSliderButton::DrawSqueeze()
{
	bool isShow = (stat_ & STAT::VISIBLE);
	FRectF3* pf = new FRectF3();

	if ( !isShow )
	{
		FRectF3 f;
		f.lastrc = rcFilter_;
		f.trc = rcFilter_;

		f.frc = rcFilter_;
		f.frc.Offset(-rcFilter_.Width(), 0);
		f.obj = this;
		
		*pf = f;


		SetRect(f.frc);
		Visible();
	}
	else
	{
		FRectF3 f;
		
		f.trc = rcFilter_;
		f.trc.Offset(-rcFilter_.Width(), 0);

		f.lastrc = f.trc;
		f.frc = rc_;
		f.obj = this;

		*pf = f;		
	}

	DWORD dw;
	CreateLightThread(_anime, pf, &dw);
}

//////////////////////////////////////////////////////////////////////////


int V4::RightButtonFloatMenu(FPointF pt, D2DControl* ctrl, std::vector<MenuItem>& items, ColorF backclr)
{
	FRectF rc(pt, FSizeF(100,100));
	D2DSliderButton* m = new D2DSliderButton();
	m->Create( ctrl->GetParentControl(), rc, VISIBLE, NONAME,-1);
	m->Set(ctrl, items);

	ctrl->GetParentControl()->SetCapture(m);

	return 0;
}