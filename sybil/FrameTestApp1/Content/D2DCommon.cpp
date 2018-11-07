#include "pch.h"
#include "D2DContext.h"
#include "D2DCommon.h"

namespace V4 {


void FillRectangle( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br )
{
	cxt->FillRectangle( rc, br );
}

void CenterTextOut( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* br  )
{
	auto old = tf->GetTextAlignment();
	tf->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER );	
	p->DrawText( str, length, tf, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	tf->SetTextAlignment( old );
}


std::wstring Format(LPCWSTR fm, ... )
{
	std::wstring rval;

	if ( fm )
	{
		va_list ap;
		va_start(ap, fm);			
		size_t len = _vscwprintf(fm,ap)+1;

		WCHAR cb[256];

		WCHAR* buffer = cb;
				
		if ( len > 256 )
			buffer = new WCHAR[len];

		int nWritten = _vswprintf_l(buffer,len, fm,0,ap);

		if (nWritten > 0)
		{					
			buffer[len-1] = 0;
			rval = buffer;
		}
				
		va_end(ap);
				
		if ( buffer != cb )
			delete [] buffer;
	}
	return rval;
}
void Trace( LPCWSTR fm, ... )
{
		{
			if ( fm )
			{
				va_list ap;
				va_start(ap, fm);			
				size_t len = _vscwprintf(fm,ap)+1;

				
				WCHAR* b = new WCHAR[len];
				WCHAR* buffer = b;


				int nWritten = _vswprintf_l(buffer,len, fm,0,ap);

				if (nWritten > 0)
				{					
					buffer[len-1] = 0;
					
				}
				
				va_end(ap);
				
				::OutputDebugString( buffer );

				delete [] b;
			}
		}
}
std::wstring str_remove( const std::wstring& str , int s, int e )
{
	if ( s == e )
		return str;

	_ASSERT( s < (int)str.length() );
	_ASSERT( e <= (int)str.length() );


	auto s1 = str.substr( 0, s );

	if ( str.length()-e > 0 )
	{
		auto s2 = str.substr( e, str.length()-e );
		return s1+s2;
	}
	return s1;
}
std::wstring str_append( const std::wstring& str , int s, WCHAR ch )
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
std::wstring CStrHex( DWORD dw )
{	
	return V4::Format(L"0x%x", dw );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2D1_RECT_F V4::ScrollbarRect( D2DScrollbarInfo& info, int typ )
{
	if ( typ == 0 )
		return info.rc;
	
	if ( info.bVertical )
	{
	
		if ( typ == 1 ) // top button
		{
			FRectF rc = info.rc;
			rc.bottom = rc.top + info.button_height;
			return rc;
		}
		else if ( typ == 2 ) // button button
		{
			FRectF rc = info.rc;
			rc.bottom = info.rc.bottom;
			rc.top = info.rc.bottom - info.button_height;		
			return rc;
		}
		else if ( typ == 3 ) // center button
		{
			float min_thumb = 16; // 最少thumb
			float y1 = info.rc.bottom-info.rc.top;
			float y2 = info.total_height + info.row_height - y1; // スクロールしなければならないサイズ(info.row_heightはあそび）

			float thumb_height = 0; //センターのサムボタンの高さ 

			float rto = 1.0f;

			if ( y2 <= 0 )
			{
				// scrollbar 不要
				thumb_height = y1 - info.button_height*2;
			}
			else if ( y1-y2-info.button_height*2-min_thumb >0 )
			{
				thumb_height = y1-y2-info.button_height*2;
				
				
			}
			else
			{
				// 最少のthmb高さ
				thumb_height = min_thumb;

				float y2a = y1-thumb_height-info.button_height*2;
				rto = y2a / y2;
								
			}
		
			info.thumb_step_c = rto;
		
			FRectF thumbrc = info.rc;		
			thumbrc.top = info.position +  info.rc.top + info.button_height;
			thumbrc.bottom = thumbrc.top + thumb_height;

			if ( thumbrc.bottom > info.rc.bottom-info.button_height )
			{
				thumbrc.bottom = info.rc.bottom-info.button_height;
				thumbrc.top = thumbrc.bottom - thumb_height;

			}

			info.rowno = (int)((info.position / info.thumb_step_c)/info.row_height);

			float h = thumbrc.Height();
			return thumbrc;
		}
		
	}
	else
	{
		float button_width = info.button_height;
		if ( typ == 1 )
		{
			FRectF rc = info.rc;
			rc.right = rc.left + button_width;
			return rc;
		}
		else if ( typ == 2 )
		{
			FRectF rc = info.rc;
			rc.right = info.rc.right;
			rc.left = rc.right - button_width;		
			return rc;
		}
		else if ( typ == 3 )
		{
			float min_thumb = 16; // 最少thumb
			float w1 = info.rc.right-info.rc.left;
			float w2 = info.total_height - w1;

			float thumb_height = w1 - info.button_height*2;
			float rto = 1.0f;

			if ( w2 <= 0 )
			{
				// scrollbar 不要
			}
			else if ( w1-w2-info.button_height*2-min_thumb >0 )
			{
				thumb_height = w1-w2-info.button_height*2;
			}
			else
			{
				thumb_height = min_thumb;
				float ywa = w1-thumb_height-info.button_height*2;

				rto = ywa / w2;

			}
		
			info.thumb_step_c = rto;
		
			FRectF thumbrc = info.rc;		
			thumbrc.left = info.position +  info.rc.left + info.button_height;
			thumbrc.right = thumbrc.left + thumb_height;

			if ( thumbrc.right > info.rc.right-info.button_height )
			{
				thumbrc.right = info.rc.right-info.button_height;
				thumbrc.left = thumbrc.left - thumb_height;

			}
			float h = thumbrc.Width();
			return thumbrc;
		}


	}
	return FRectF(0,0,0,0);
}
void V4::DrawScrollbar( ID2D1RenderTarget* cxt, D2DScrollbarInfo& info )
{
	ComPTR<ID2D1SolidColorBrush> bkcolor,br1b,br2;

	bkcolor = info.clr[0];
	br1b = info.clr[1];
	br2 = info.clr[2];

	//cxt->CreateSolidColorBrush(D2RGBA(230,230,230,255 ), &bkcolor );
	//cxt->CreateSolidColorBrush(D2RGBA(200,200,200,255 ), &br1b );
	//cxt->CreateSolidColorBrush(D2RGBA(100,100,100,255 ), &br2 );
	
	cxt->FillRectangle( info.rc, bkcolor ); // 全体

	
	if ( info.bVertical )
	{
		// Upper button
		FRectF rc = ScrollbarRect( info, 1 );
				
		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3, info.rc.top+5 ) );				
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 0 );		
		

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3, info.rc.bottom-10 ) );				

		// Lower button
		rc = ScrollbarRect( info, 2 );		
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 2 );		

		cxt->SetTransform( &mat ); // 戻す



		// Thumb button
		rc = ScrollbarRect( info, 3 );
		rc.Offset( 1, 0 );
		

		info.thumb_rc = rc;

		bool bl = (bool)((info.stat & D2DScrollbarInfo::ONTHUMB) || (info.stat & D2DScrollbarInfo::CAPTURED));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	else
	{
		// left side button
		FRectF rc = ScrollbarRect( info, 1 );
				
		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+5, info.rc.top+5 ) );				
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 3 );		
		
		cxt->SetTransform( mat.CalcOffset( info.rc.right-10, info.rc.top+5 ) );				

		// right side button
		rc = ScrollbarRect( info, 2 );		
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 1 );		

		cxt->SetTransform( &mat ); // 戻す

		// Thumb button
		rc = ScrollbarRect( info, 3 );		
		rc.Offset( 0, 1 );

		info.thumb_rc = rc;

		bool bl = (bool)(((info.stat & D2DScrollbarInfo::ONTHUMB) > 0) | ((info.stat & D2DScrollbarInfo::CAPTURED) >0));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	
}
}


#include "higgsjson.h"
using namespace HiggsJson;
bool V4::MenuItemsJsonParse( LPCWSTR json, V4::D2DMenuItem** head, int* itemscnt )
{
	// [{"name":"hoi", id:1}, {"name":"hoi2", id:2}]

	std::vector<V4::D2DMenuItem> mar;

	std::vector<Higgs> ar;
	ParseList( json, ar );

	for( auto& it : ar )
	{
		V4::D2DMenuItem r;

		auto m = ToMap(it);

		r.name = ::SysAllocString(ToStr(m[L"name"]).c_str());
		r.id = ToInt(m[L"id"]);

		mar.push_back(r);
	}


	int cnt = (int)mar.size();

	V4::D2DMenuItem* h = new V4::D2DMenuItem[cnt];
	for( int i = 0; i < cnt; i++ )
	{
		h[i].id = mar[i].id;
		h[i].name = mar[i].name;
	}

	*head = h;
	*itemscnt = cnt;

	return true;
}
void V4::MenuItemsClose( V4::D2DMenuItem* head, int itemscnt )
{
	V4::D2DMenuItem* k = head;
	for( int i = 0; i < itemscnt; i++ )
	{		
		V4::D2DMenuItem* it = k;
		::SysFreeString(it->name);

		k++;
	}

	delete [] head;
}
