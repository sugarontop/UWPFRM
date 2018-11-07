#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DInputControl.h"
#include "..\sybil\sybil.h"
#include "Entry.h"
#include "BSTRptr.h"
#include "HiggsJson.h"

using namespace V4;
using namespace V4_XAPP1;
using namespace sybil;

void CSV4Point(std::vector<Rousoku>& ar,  BSTR data, bool header, float* vmax, float* vmin);
std::vector<Rousoku> HeikinAsi(const std::vector<Rousoku>& ar);
 LPCSTR ToUtf8( LPCWSTR str, int* pcblen )
{
	int& cblen = *pcblen;
	cblen = ::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), 0,0,0,0);
	
	byte* cb = new byte[cblen+1];

	::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), (LPSTR)cb,cblen,0,0);

	cb[cblen]=0;

	return (LPCSTR)cb;
}


struct Macd
{
	float B; // m4e
	float C; // 12 avg
	float D; // 26 avg
	float E; // macd
	float F; // signal
	float G; // histomgram
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

D2DCells::D2DCells()
{
}
		
void D2DCells::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR font, int fontheight, Script* sc)
{
	D2DWindow* win = pacontrol->GetParentWindow();
	InnerCreateWindow(win,pacontrol,rc,stat, NONAME, -1);



	fontnm_ = font;
	font_height_ = (float)fontheight;
	padding_l_=padding_h_=0;
	sc_ = sc;

	child_rc_.SetRect(300,10, FSizeF(400,250));

}
int D2DCells::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			mat.Offset(padding_l_, padding_h_);

			OnPaint(cxt);


			mat.PopTransform();
		}
		break; 
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);
			
			if ( rc_.PtInRect(pt))
			{
				pt.x = pt.x-rc_.left-padding_l_;
				pt.y = pt.y-rc_.top-padding_h_;

				
				BSTRPtr bs;
				std::wstring cd;
				for( auto& it : rows_ )
				{
					if ( it.rc2.PtInRect(pt) )
					{
						cd = it.val;

						std::wstring cmd = L"GetUrl('" ;
						cmd += cd;
						cmd += L"');";

						sc_->ExecBSTR( cmd.c_str(), &bs );
						break;
					}
				}
				

				D2DChartView* pview = new D2DChartView();
				pview->Create( GetParentControl(), child_rc_, STAT::VISIBLE );
				pview->TitleName(cd,0);
				pview->Load(bs);

				//auto rc1 = child_rc_;
				//rc1.Offset( rc1.Width()+5, 0);
				//D2DChartView* pview2 = new D2DChartView();
				//pview2->Create( GetParentControl(), rc1, STAT::VISIBLE );
				//pview2->TitleName(cd,1);
				//pview2->Load(bs);



				child_rc_.Offset( 0, child_rc_.Height()+5);


				d->redraw();
				ret = 1;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt = mat_.DPtoLP(lp);			
			if ( rc_.PtInRect(pt))
			{
				pt.x = pt.x-rc_.left-padding_l_;
				pt.y = pt.y-rc_.top-padding_h_;

				for( auto& it : rows_ )
				{					
					it.stat = ( it.rc2.PtInRect(pt) ? 1 : 0 );
				}

				d->redraw();

				ret = 1;
			}
		}
		break;
	}

	return ret;

}
void D2DCells::InnerSetting( void* p )
{
	auto& m = *static_cast<std::map<std::wstring, HiggsJson::Higgs>*>( p );
	
	auto itf = m.find(L"_font");
	if ( itf != m.end())
	{
		fontnm_ = HiggsJson::ToStr((*itf).second);
		m.erase(itf);
	}



	float* f[]= { &font_height_, &padding_l_, &padding_h_ };

	LPCWSTR k[]={L"_font_size",L"_padding_h",L"_padding_l"};

	for( int i = 0; i < 3; i++ )
	{
		itf = m.find(k[i]);
		if ( itf != m.end())
		{
			auto s = HiggsJson::ToStr((*itf).second);
			*f[i] = (float)_wtof(s.c_str());
			m.erase(itf);
		}
	}
	
	width1_ = 100.0f;
	width2_ = 100.0f;

	itf = m.find(L"_cell_width");
	if ( itf != m.end())
	{
		auto ls = HiggsJson::ToList((*itf).second);

		width1_ = (float)HiggsJson::ToDouble(ls[0]);
		width2_ = (float)HiggsJson::ToDouble(ls[1]);
			   
		m.erase(itf);
	}
}

void D2DCells::SetData( BSTR json )
{
	std::map<std::wstring, HiggsJson::Higgs> m;
	HiggsJson::ParseMap(json,  m);

	InnerSetting(&m);

	ComPTR<IDWriteTextFormat> tf, vf;

	auto wf = parent_->cxt()->wfactory;

	auto hr = wf->CreateTextFormat( fontnm_.c_str(), nullptr, 
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		font_height_,
		DEFAULTLOCALE,
		&tf );
	
	if ( hr != S_OK ) 
		return ;
	
	

	int i = 0;
	rows_.clear();
	rows_.resize(m.size());

	FRectF rc1(0,0,width1_,30);
	FRectF rc2(width1_,0,width2_,30);

	for( auto& it : m )
	{
		ComPTR<IDWriteTextLayout> tl,tl2;

		auto& title = it.first;
		auto val = HiggsJson::ToStr(it.second);


		wf->CreateTextLayout( title.c_str(), title.length(), tf, 100,1000, &tl ); 
		wf->CreateTextLayout( val.c_str(), val.length(), tf, 100,1000, &tl2 ); 

		Row r;
		r.title = tl;
		r.value = tl2;
		r.stat = 0;

		r.rc1 = rc1;
		r.rc2 = rc2;

		rc1.Offset( 0, rc1.Height());
		rc2.Offset( 0, rc2.Height());
		r.val = val;

		rows_[i++] = r;
	}
}

void D2DCells::Reset()
{
	rows_.clear();
}

void D2DCells::OnPaint(D2DContext& cxt)
{
	for( auto& it : rows_ )
	{
		FPointF pt = it.rc1.LeftTop();
		FPointF pt1 = it.rc2.LeftTop();
		cxt.cxt->DrawTextLayout( pt, it.title, cxt.white );
		cxt.cxt->DrawTextLayout( pt1, it.value, (it.stat == 0 ? cxt.white : cxt.red) );
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////




static void completefunc(void* p)
{
	ResponseData* xrd = (ResponseData*)p;

	int res = xrd->result;


	WParameter* wp = (WParameter*)xrd->option;

	D2DWindow* win = (D2DWindow*)wp->prm;
	
	wp->prm = xrd;

	win->PostMessage( WM_D2D_APP_INTERNET_GET, (INT_PTR)wp, nullptr );

}

D2DChartView::D2DChartView()
{

}
		
void D2DChartView::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat)
{
	D2DWindow* win = pacontrol->GetParentWindow();
	InnerCreateWindow(win,pacontrol,rc,stat, L"D2DChartView", -1);

}

void D2DChartView::TitleName( std::wstring nm, int typ )
{
	ComPTR<IDWriteTextFormat> tf, vf;

	float font_height = 50.0f;
	auto wf = parent_->cxt()->wfactory;

	auto hr = wf->CreateTextFormat( L"Arial", nullptr, 
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		font_height,
		DEFAULTLOCALE,
		&tf );


	if ( title_ )
		title_->Release();

	wf->CreateTextLayout( nm.c_str(), nm.length(), tf, 9000,9000, &title_ ); 
	stock_cd_ = nm;
	chart_typ_ = typ;
}


void D2DChartView::Load( LPCWSTR url )
{
	::OutputDebugString( url );
	
	
	WParameter* wp = new WParameter();
	wp->sender = this;
	wp->target = this;
	wp->prm = GetParentWindow();


	BSTRPtr burl = url;
	

	ResponseData* rd = new ResponseData();
	ResponseDataInit(rd);		
	rd->option = wp;
			   
	int seqno = GETInternet( burl, nullptr,0, rd, completefunc );	

}


int D2DChartView::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			//mat.Offset(padding_l_, padding_h_);

			OnPaint(cxt);


			mat.PopTransform();
		}
		break; 		
		case WM_D2D_APP_INTERNET_GET:
		{
			WParameter* wpm = (WParameter*)wp;

			if ( wpm->target == this )
			{
				int len;
				LPCSTR data_utf8 = nullptr;
				sybil::ResponseData* xrd = (sybil::ResponseData*)wpm->prm;
				int res = xrd->result;
				if ( res == 200 )
				{
					BSTR bs = xrd->data;
					
					
					std::vector<Rousoku> r, r2;

					CSV4Point( r, bs, true, &vmax, &vmin );

					if ( chart_typ_ == 1 )					
						r2 = HeikinAsi(r);
					else if ( chart_typ_ == 0 )
						r2 = r;


					int i =0;
					ar_.resize(r2.size());
					for( auto& it : r2 )
					{
						ar_[i++].r_ = it;
					}

					data_utf8 = ToUtf8( bs, &len );	

					d->redraw();
				}

				sybil::ResponseDataClear(xrd);
				delete xrd;
				delete wpm;



				/*{
					auto fnm1 = stock_cd_;
					fnm1 += L".csv";
					bool bl = sybil::WriteFile( fnm1.c_str(), (byte*)data_utf8, len, 1 );
				}*/

				delete data_utf8;
				return 1;
			}
		}
		break;
	}
	return ret;
}

void ChartMat( D2DMat& src, int rousoku_cnt, float xdev, float vmax, float vmin, float ydev )
{
	src._11 = src._11 * (float)xdev/(rousoku_cnt*ROSOKU_UNIT);
	src._22 = -src._22* ydev/(vmax-vmin);
	src._32 = src._32 + (ydev)*(vmax-0)/(vmax-vmin);
}


void D2DChartView::OnPaint(D2DContext& cxt)
{
	
	cxt.cxt->FillRectangle( rc_.ZeroRect(), cxt.white );
	
	
	if ( title_ )
		cxt.cxt->DrawTextLayout( FPointF(0,0), title_, cxt.ltgray );

	D2DMatrix mat(cxt);
	matchart_ = mat.PushTransform();

		const int cnt = 100;
		
		mat.PushTransform();
		{
			ChartMat(mat, cnt, rc_.Width(), vmax, vmin, rc_.Height());
			matchart_ = mat;

			mat.SetTransform();

			int s = max(0, ar_.size()-cnt);
	
			for(UINT i = s; i < ar_.size(); i++)
			{
				ar_[i].Draw(cxt);

				mat.Offset(ROSOKU_UNIT,0);
			}
		}
		mat.PopTransform();
		

		FPointF ptt[5], pttt[5];

		mat.PushTransform();
		{
			ChartMat(mat, cnt, rc_.Width(), vmax, vmin, rc_.Height());
			mat.SetTransform();
			float step = (vmax-vmin)/5;

			for( int i = 1; i < 5; i++ )
			{
				FPointF pt1(0, step*i+vmin);
				FPointF pt2(cnt*ROSOKU_UNIT , pt1.y);

				ptt[i] = mat.LPtoDP(pt1);
				pttt[i] = mat.LPtoDP(pt2);
			}
		}
		mat.PopTransform();


		//cxt.cxt->FillRectangle( FRectF(-5,-5,5,5), cxt.red);

		

		for( int i = 1; i < 5; i++ )
		{			
			ptt[i] = mat.DPtoLP(ptt[i]);
			pttt[i] = mat.DPtoLP(pttt[i]);
			cxt.cxt->DrawLine( ptt[i], pttt[i], cxt.black, 1.0, cxt.dot2_ );
		}

	
	

	mat.PopTransform();




}	

std::vector<std::wstring> Split( LPCWSTR str, LPCWSTR split )
{
    std::vector<std::wstring> ar;
    int splen = wcslen(split);
    int len = wcslen(str);
    _ASSERT( 0 < splen && splen <= 2  );

    int si = 0;
    for( int i = 0; i < len; i++ )
    {
        if ( str[i] == split[0] || (i == len && 0 < len) )
        {
            if (splen == 1 || (splen == 2 && (str[i+1] == split[1] || i == len)) )
            {
                std::wstring s( &str[si], i-si );
                ar.push_back( s );
                si = i + splen;
            }
        }       
    }
    return ar;
}


void CSV4Point(std::vector<Rousoku>& ar,  BSTR data,  bool header, float* vmax, float* vmin)
{
	ar.clear();
	
	Rousoku r;

	auto rows = Split( data, L"\r\n");

	int i = (header ? 0 : 1);

	*vmax = 0.0f;
	*vmin = 99999999.0f;

	for( auto& row : rows )
	{
		if ( i != 0 )
		{
			auto cells = Split( row.c_str(), L",");

			if (!cells.empty())
			{
				r.m1 = (float)_wtof(cells[1].c_str());
				r.m2 = (float)_wtof(cells[2].c_str());
				r.m3 = (float)_wtof(cells[3].c_str());
				r.m4 = (float)_wtof(cells[4].c_str());


				*vmax = max(*vmax, r.m2 );
				*vmin = min(*vmin, r.m3 );

				ar.push_back(r);
			}
		}
		i++;		
	}

	std::reverse(ar.begin(), ar.end());
}





std::vector<Rousoku> HeikinAsi(const std::vector<Rousoku>& ar)
{
	std::vector<Rousoku> ret(ar.size());

	auto it = ar.begin();

	Rousoku pr = (*it++);

	int i = 0;
	for( ; it != ar.end(); it++ )
	{
		Rousoku r = (*it);
	
		r.m1 = (i==0 ? ((pr.m1+pr.m2+pr.m3+pr.m4)/4) : ((pr.m1+pr.m4)/2));			
		r.m4 = (r.m1+r.m2+r.m3+r.m4)/4;

		ret[i++] = r;
		pr = r;		
	}
	return ret;
}

std::vector<Macd> MACD( const std::vector<Rousoku>& ar )
{
	std::vector<Macd> ret;
	
	const int T1 = 12;
	const int T2 = 9;
	const int T3 = 26;

	float sumb = 0;

	int md = 0;
	Macd macd,prmacd;

	const float B1 = (2.0f/(T1+1));
	const float C1 = (1.0f-(2.0f/(T1+1)));

	for( int i=0; i < (int)ar.size(); i++ )
	{
		macd.B = ar[i].m4;
		macd.C = 0;

		if ( -1 < i-T1+1  )
		{			
			if ( md == 0 )
				md = 1;
			else
				sumb -= ar[i-T1+1].m4;
		}

		sumb += ar[i].m4;

		if ( md == 1 )
		{
			macd.C = sumb/T1;
			md = 2;
		}
		else if ( md == 2 )
		{
			auto& pr = prmacd;
			macd.C = macd.B*B1+ pr.C*C1;
		}
		
		ret.push_back(macd);
		prmacd = macd;
	}

	md =0;
	sumb = 0;

	const float B2 = (2.0f/(T3+1));
	const float D2 = (1.0f-(2.0f/(T3+1)));

	for( int i=0; i < (int)ar.size(); i++ )
	{
		if ( -1 < i-T3+1  )
		{			
			if ( md == 0 )
				md = 1;
			else
				sumb -= ar[i-T3+1].m4;
		}

		sumb += ar[i].m4;

		macd = ret[i];

		if ( md == 1 )
		{
			macd.D = sumb/T3;
			md = 2;
		}
		else if ( md == 2 )
		{
			auto& pr = prmacd;
			macd.D = macd.B*B2+ pr.D*D2;
		}

		macd.E = macd.C-macd.D;		
		ret[i] = macd;
		prmacd = macd;
	}


	md =0;
	sumb = 0;

	const float E3 = (2.0f/(T2+1));
	const float F3 = (1.0f-(2.0f/(T2+1)));

	for( int i=(T3-1); i < (int)ret.size(); i++ )
	{
		macd = ret[i];
	
		if ( -1 < (i-(T3-1))-T2+1  )
		{
			if ( md == 0 )
				md = 1;
			else
				sumb -= ret[i-T2+1].E;	
		}

		sumb += ret[i].E;

		if ( md == 1 )
		{
			macd.F = sumb/T2;
			md = 2;
		}
		else if ( md == 2 )
		{
			auto& pr = prmacd;
			macd.F = macd.E*E3+ pr.F*F3;
		}

		macd.G = macd.E-macd.F;

		ret[i] = macd;
		prmacd = macd;
	}

	ret.erase(ret.begin(), ret.begin()+(T2+T3-2));

	return ret;
}